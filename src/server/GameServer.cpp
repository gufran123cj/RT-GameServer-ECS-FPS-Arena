#include "GameServer.hpp"
#include "../network/Packet.hpp"
#include "../network/PacketTypes.hpp"
#include "../core/systems/MovementSystem.hpp"
#include "../core/components/HealthComponent.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/ShootingSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include <LDtkLoader/Project.hpp>
#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>

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
    
    // Check for dead players (health <= 0) and disconnect them
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && conn.entity.isValid()) {
            auto* healthComp = world.getComponent<game::core::components::HealthComponent>(conn.entity.id);
            if (healthComp && healthComp->isDead()) {
                std::cout << "Player " << conn.entity.id << " is dead! Disconnecting..." << std::endl;
                // Destroy entity
                world.destroyEntity(conn.entity);
                // Disconnect client
                networkManager.handleDisconnect(addr);
                // Send disconnect packet to client
                game::network::Packet disconnectPacket(game::network::PacketType::DISCONNECT);
                disconnectPacket.setSequence(1);
                disconnectPacket.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count()));
                networkManager.sendPacket(addr, disconnectPacket);
            }
        }
    }
    
    // CRITICAL: Handle new connections and spawn entities
    // Each client gets its own unique entity
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && !conn.entity.isValid()) {
            // New client, spawn entity at initial position from client
            sf::Vector2f initialPos = networkManager.getClientInitialPosition(addr);
            game::core::Entity entity = spawnPlayer(addr, initialPos);
            
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
    
    // Add components
    // Use initial position from client (LDtk player position) if provided, otherwise use default
    float spawnX, spawnY;
    if (initialPosition.x != 0 || initialPosition.y != 0) {
        // Use client's initial position (LDtk player position)
        spawnX = initialPosition.x;
        spawnY = initialPosition.y;
    } else {
        // Default spawn position (fallback)
        spawnX = 100.0f + (networkManager.getClientCount() * 50.0f);
        spawnY = 100.0f;
    }
    
    game::core::components::PositionComponent posComp(spawnX, spawnY);
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

} // namespace game::server

