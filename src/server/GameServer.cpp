#include "GameServer.hpp"
#include "../network/Packet.hpp"
#include "../network/PacketTypes.hpp"
#include "../core/systems/MovementSystem.hpp"
#include "../core/components/HealthComponent.hpp"
#include "../core/components/KillCounterComponent.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/ShootingSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "CollisionHelper.hpp"
#include <LDtkLoader/Project.hpp>
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>
#include <random>

namespace game::server {

GameServer::GameServer() 
    : running(false)
    , accumulator(0.0f) {
}

GameServer::~GameServer() {
    shutdown();
}

bool GameServer::initialize(const ServerConfig& cfg) {
    config = cfg;
    
    // Initialize network
    if (!networkManager.initialize(config.port)) {
        return false;
    }
    
    // Load colliders (static obstacles)
    loadColliders();
    
    // Initialize world and register systems
    // IMPORTANT: System execution order (by priority):
    // - ShootingSystem: 10 (processes SHOOT packets, spawns projectiles)
    // - CollisionSystem: 50 (checks collisions before movement)
    // - ProjectileSystem: 75 (updates projectile lifetime, checks collisions)
    // - MovementSystem: 100 (updates positions based on velocity)
    world.registerSystem(std::make_unique<systems::ShootingSystem>(networkManager));
    world.registerSystem(std::make_unique<systems::CollisionSystem>(colliders));
    world.registerSystem(std::make_unique<systems::ProjectileSystem>(colliders));
    world.registerSystem(std::make_unique<game::core::systems::MovementSystem>());
    world.initialize();
    
    running = true;
    lastUpdateTime = std::chrono::steady_clock::now();
    lastSnapshotTime = lastUpdateTime;
    
    std::cout << "GameServer initialized:" << std::endl;
    std::cout << "  Port: " << config.port << std::endl;
    std::cout << "  Tick Rate: " << config.tickRate << " Hz" << std::endl;
    std::cout << "  Snapshot Rate: " << config.snapshotRate << " Hz" << std::endl;
    std::cout << "  Max Players: " << config.maxPlayers << std::endl;
    
    return true;
}

void GameServer::shutdown() {
    if (!running) return;
    
    running = false;
    networkManager.shutdown();
    world.shutdown();
    
    std::cout << "GameServer shutdown" << std::endl;
}

void GameServer::run() {
    std::cout << "GameServer running..." << std::endl;
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration<float>(
            currentTime - lastUpdateTime
        ).count();
        lastUpdateTime = currentTime;
        
        // Clamp frame time to prevent spiral of death
        frameTime = std::min(frameTime, 0.1f);
        
        // Process network packets
        processNetwork();
        
        // Fixed timestep update
        accumulator += frameTime;
        const float fixedDelta = config.fixedTimestep();
        
        while (accumulator >= fixedDelta) {
            updateGame(fixedDelta);
            accumulator -= fixedDelta;
        }
        
        // Send snapshots (at snapshot rate)
        auto snapshotElapsed = std::chrono::duration<float>(
            currentTime - lastSnapshotTime
        ).count();
        
        if (snapshotElapsed >= config.snapshotInterval()) {
            sendSnapshots();
            lastSnapshotTime = currentTime;
        }
        
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GameServer::stop() {
    running = false;
}

void GameServer::processNetwork() {
    // Process incoming packets
    networkManager.processPackets();
    
    // CRITICAL: Process INPUT packets from clients
    // Each client's INPUT should ONLY affect their own entity
    // IMPORTANT: If no INPUT is received, set velocity to zero (stop movement)
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && conn.entity.isValid()) {
            auto input = networkManager.getLastInput(addr);
            auto* velComp = world.getComponent<game::core::components::VelocityComponent>(conn.entity.id);
            
            if (input.valid) {
                // Read input data from packet
                game::network::Packet& packet = input.packet;
                packet.resetRead();  // Skip header
                
                float velX = 0, velY = 0;
                if (packet.read(velX) && packet.read(velY)) {
                    if (velComp) {
                        velComp->velocity.x = velX;
                        velComp->velocity.y = velY;
                    }
                }
            } else {
                // No INPUT received - stop movement for this entity
                if (velComp) {
                    velComp->velocity.x = 0.0f;
                    velComp->velocity.y = 0.0f;
                }
            }
        }
    }
    
    // Check for connection timeouts
    networkManager.checkTimeouts(config.connectionTimeout);
    
    // Check for dead players (health <= 0) and respawn them
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && conn.entity.isValid()) {
            auto* healthComp = world.getComponent<game::core::components::HealthComponent>(conn.entity.id);
            if (healthComp && healthComp->isDead()) {
                std::cout << "Player " << conn.entity.id << " is dead! Respawning..." << std::endl;
                
                // Find a safe spawn position
                sf::Vector2f safeSpawnPos = findSafeSpawnPosition();
                
                // Respawn player at safe position
                respawnPlayer(conn.entity.id, safeSpawnPos);
            }
        }
    }
    
    // CRITICAL: Handle new connections and spawn entities
    // Each client gets its own unique entity
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && !conn.entity.isValid()) {
            // New client, spawn entity at random safe position (ignore initial position from client)
            game::core::Entity entity = spawnPlayer(addr, sf::Vector2f(0, 0));  // initialPosition ignored
            
            networkManager.setClientEntity(addr, entity);
            networkManager.sendConnectAck(addr, entity.id);
        }
    }
}

void GameServer::updateGame(float deltaTime) {
    // Update ECS world
    world.update(deltaTime);
}

void GameServer::sendSnapshots() {
    if (networkManager.getClientCount() == 0) {
        return;  // No clients to send to
    }
    
    game::network::Packet packet(game::network::PacketType::SNAPSHOT);
    createSnapshotPacket(packet);
    
    // Broadcast to all clients
    networkManager.broadcastPacket(packet);
}

game::core::Entity GameServer::spawnPlayer(const game::network::Address& address, const sf::Vector2f& initialPosition) {
    // Create entity
    game::core::Entity entity = world.createEntity();
    
    // Always use random safe spawn position (ignore initialPosition from client)
    sf::Vector2f safeSpawnPos = findSafeSpawnPosition();
    
    game::core::components::PositionComponent posComp(safeSpawnPos.x, safeSpawnPos.y);
    world.addComponent<game::core::components::PositionComponent>(entity.id, posComp);
    
    game::core::components::VelocityComponent velComp(0.0f, 0.0f);
    world.addComponent<game::core::components::VelocityComponent>(entity.id, velComp);
    
    // Use same player size as client
    const sf::Vector2f PLAYER_SIZE = {3.0f, 5.0f};
    game::core::components::SpriteComponent spriteComp(PLAYER_SIZE, sf::Color::Green);
    world.addComponent<game::core::components::SpriteComponent>(entity.id, spriteComp);
    
    // Add HealthComponent (10 health)
    game::core::components::HealthComponent healthComp(10.0f);
    world.addComponent<game::core::components::HealthComponent>(entity.id, healthComp);
    
    // Add KillCounterComponent
    game::core::components::KillCounterComponent killCounter;
    world.addComponent<game::core::components::KillCounterComponent>(entity.id, killCounter);
    
    return entity;
}

void GameServer::createSnapshotPacket(game::network::Packet& packet) {
    // Get all entities with Position + Sprite components
    auto entities = world.getEntitiesWith<
        game::core::components::PositionComponent,
        game::core::components::SpriteComponent
    >();
    
    // Write entity count
    uint32_t entityCount = static_cast<uint32_t>(entities.size());
    packet.write(entityCount);
    
    // Write each entity
    for (game::core::Entity::ID entityID : entities) {
        // Write entity ID
        packet.write(entityID);
        
        // Write PositionComponent
        const auto* pos = world.getComponent<game::core::components::PositionComponent>(entityID);
        if (pos) {
            packet.write(pos->position.x);
            packet.write(pos->position.y);
        }
        
        // Write SpriteComponent (size and color)
        const auto* sprite = world.getComponent<game::core::components::SpriteComponent>(entityID);
        if (sprite) {
            packet.write(sprite->size.x);
            packet.write(sprite->size.y);
            packet.write(sprite->color.r);
            packet.write(sprite->color.g);
            packet.write(sprite->color.b);
            packet.write(sprite->color.a);
        }
        
        // Write HealthComponent (if exists)
        const auto* health = world.getComponent<game::core::components::HealthComponent>(entityID);
        if (health) {
            packet.write(static_cast<uint8_t>(1));  // Has health component flag
            packet.write(health->currentHealth);
            packet.write(health->maxHealth);
        } else {
            packet.write(static_cast<uint8_t>(0));  // No health component
        }
        
        // Write KillCounterComponent (if exists)
        const auto* killCounter = world.getComponent<game::core::components::KillCounterComponent>(entityID);
        if (killCounter) {
            packet.write(static_cast<uint8_t>(1));  // Has kill counter flag
            packet.write(static_cast<int32_t>(killCounter->getKills()));
        } else {
            packet.write(static_cast<uint8_t>(0));  // No kill counter
        }
    }
}

void GameServer::loadColliders() {
    colliders.clear();
    
    try {
        // Load LDtk project (same file as client)
        ldtk::Project project;
        std::string ldtk_filename = "assets/maps/map.ldtk";
        project.loadFromFile(ldtk_filename);
        
        // Get the world and level
        auto& world = project.getWorld();
        auto& level0 = world.getLevel("World_Level_0");
        
        // Load colliders from IntGrid "Collisions" layer
        auto& collisions_layer = level0.getLayer("Collisions");
        if (collisions_layer.getType() == ldtk::LayerType::IntGrid) {
            // Manually iterate through all grid cells to find walls (value = 1)
            int gridSize = collisions_layer.getCellSize();
            const auto& gridSize_pt = collisions_layer.getGridSize();
            int gridWidth = gridSize_pt.x;
            int gridHeight = gridSize_pt.y;
            
            int wallCount = 0;
            // Check each grid cell
            for (int y = 0; y < gridHeight; ++y) {
                for (int x = 0; x < gridWidth; ++x) {
                    try {
                        const auto& intGridVal = collisions_layer.getIntGridVal(x, y);
                        // Value 1 = walls
                        if (intGridVal.value == 1) {
                            float pxX = static_cast<float>(x * gridSize);
                            float pxY = static_cast<float>(y * gridSize);
                            float cellSize = static_cast<float>(gridSize);
                            
                            colliders.emplace_back(pxX, pxY, cellSize, cellSize);
                            wallCount++;
                        }
                    } catch (...) {
                        // Skip invalid cells
                        continue;
                    }
                }
            }
            
            std::cout << "Server: Loaded " << wallCount << " collision cells from IntGrid layer" << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Server WARNING: Could not load collisions from LDtk file: " << ex.what() << std::endl;
        std::cerr << "Server will run without collision detection!" << std::endl;
    }
    
    std::cout << "Server: Total colliders loaded: " << colliders.size() << std::endl;
}

void GameServer::respawnPlayer(game::core::Entity::ID entityID, const sf::Vector2f& spawnPosition) {
    // Reset position
    auto* posComp = world.getComponent<game::core::components::PositionComponent>(entityID);
    if (posComp) {
        posComp->position = spawnPosition;
    }
    
    // Reset velocity
    auto* velComp = world.getComponent<game::core::components::VelocityComponent>(entityID);
    if (velComp) {
        velComp->velocity = sf::Vector2f(0.0f, 0.0f);
    }
    
    // Reset health
    auto* healthComp = world.getComponent<game::core::components::HealthComponent>(entityID);
    if (healthComp) {
        healthComp->currentHealth = healthComp->maxHealth;
    }
    
    std::cout << "Player " << entityID << " respawned at (" << spawnPosition.x << ", " << spawnPosition.y << ")" << std::endl;
}

sf::Vector2f GameServer::findSafeSpawnPosition() {
    // Try to find a safe spawn position (no collision)
    // Calculate map bounds from LDtk world size
    float MAP_MIN_X = 50.0f;
    float MAP_MAX_X = 462.0f;  // 512 - 50 (padding)
    float MAP_MIN_Y = 50.0f;
    float MAP_MAX_Y = 206.0f;  // 256 - 50 (padding)
    
    // Try to get map bounds from LDtk project (same file used in loadColliders)
    try {
        ldtk::Project project;
        project.loadFromFile("assets/maps/map.ldtk");
        const auto& world = project.getWorld();
        try {
            const auto& level = world.getLevel("World_Level_0");
            // Level size is the map dimensions
            const float PADDING = 50.0f;
            MAP_MIN_X = PADDING;
            MAP_MIN_Y = PADDING;
            MAP_MAX_X = static_cast<float>(level.size.x) - PADDING;  // Map width - padding
            MAP_MAX_Y = static_cast<float>(level.size.y) - PADDING;  // Map height - padding
            
            std::cout << "Spawn bounds from LDtk: X=[" << MAP_MIN_X << ", " << MAP_MAX_X 
                      << "], Y=[" << MAP_MIN_Y << ", " << MAP_MAX_Y << "] (Map size: " 
                      << level.size.x << "x" << level.size.y << ")" << std::endl;
        } catch (const std::exception&) {
            // Level not found, use defaults
        }
    } catch (const std::exception& ex) {
        std::cerr << "WARNING: Could not load map bounds from LDtk: " << ex.what() << std::endl;
        // If LDtk load fails, calculate bounds from colliders
        if (!colliders.empty()) {
            float minX = colliders[0].left;
            float maxX = colliders[0].left + colliders[0].width;
            float minY = colliders[0].top;
            float maxY = colliders[0].top + colliders[0].height;
            
            for (const auto& collider : colliders) {
                minX = std::min(minX, collider.left);
                maxX = std::max(maxX, collider.left + collider.width);
                minY = std::min(minY, collider.top);
                maxY = std::max(maxY, collider.top + collider.height);
            }
            
            // Add padding around colliders, but ensure we stay within reasonable bounds
            const float PADDING = 50.0f;
            MAP_MIN_X = std::max(0.0f, minX - PADDING);
            MAP_MAX_X = maxX + PADDING;
            MAP_MIN_Y = std::max(0.0f, minY - PADDING);
            MAP_MAX_Y = maxY + PADDING;
            
            std::cout << "Spawn bounds from colliders: X=[" << MAP_MIN_X << ", " << MAP_MAX_X 
                      << "], Y=[" << MAP_MIN_Y << ", " << MAP_MAX_Y << "]" << std::endl;
        }
    }
    
    // Ensure valid bounds
    if (MAP_MAX_X <= MAP_MIN_X) MAP_MAX_X = MAP_MIN_X + 100.0f;
    if (MAP_MAX_Y <= MAP_MIN_Y) MAP_MAX_Y = MAP_MIN_Y + 100.0f;
    
    const sf::Vector2f PLAYER_SIZE = {3.0f, 5.0f};
    const int MAX_ATTEMPTS = 100;  // Increased attempts for better spawn position finding
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(MAP_MIN_X, MAP_MAX_X);
    std::uniform_real_distribution<float> distY(MAP_MIN_Y, MAP_MAX_Y);
    
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        sf::Vector2f candidatePos(distX(gen), distY(gen));
        
        // Check if position is safe (no collision with colliders)
        if (!CollisionHelper::wouldCollideAt(candidatePos, PLAYER_SIZE, colliders)) {
            std::cout << "Found safe spawn position: (" << candidatePos.x << ", " << candidatePos.y << ")" << std::endl;
            return candidatePos;
        }
    }
    
    // Fallback: try a few known safe positions (top-left area of map, visible on screen)
    std::vector<sf::Vector2f> fallbackPositions = {
        sf::Vector2f(100.0f, 100.0f),
        sf::Vector2f(150.0f, 100.0f),
        sf::Vector2f(200.0f, 100.0f),
        sf::Vector2f(100.0f, 150.0f),
        sf::Vector2f(150.0f, 150.0f),
        sf::Vector2f(200.0f, 150.0f),
        sf::Vector2f(250.0f, 100.0f),
        sf::Vector2f(300.0f, 100.0f)
    };
    
    for (const auto& pos : fallbackPositions) {
        if (!CollisionHelper::wouldCollideAt(pos, PLAYER_SIZE, colliders)) {
            std::cout << "WARNING: Using fallback spawn position (" << pos.x << ", " << pos.y << ")" << std::endl;
            return pos;
        }
    }
    
    // Last resort: return a safe position near top-left (should be visible)
    std::cout << "WARNING: Could not find safe spawn position, using default top-left position" << std::endl;
    return sf::Vector2f(150.0f, 100.0f);
}

} // namespace game::server

