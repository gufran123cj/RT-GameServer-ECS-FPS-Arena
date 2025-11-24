#include <iostream>
#include <chrono>
#include <thread>
#include <map>

#include <SFML/Graphics.hpp>
#include <LDtkLoader/Project.hpp>

#include "TileMap.hpp"
#include "client/ClientNetworkManager.hpp"
#include "network/Packet.hpp"
#include "network/PacketTypes.hpp"
#include "core/components/PositionComponent.hpp"
#include "core/components/SpriteComponent.hpp"


auto getPlayerCollider(sf::Shape& player) -> sf::FloatRect {
    auto bounds = player.getGlobalBounds();
    sf::FloatRect rect;
    rect.left = bounds.left;
    rect.width = bounds.width;
    rect.top = bounds.top + bounds.height / 2;
    rect.height = bounds.height / 2;
    return rect;
}

auto getColliderShape(const sf::FloatRect& rect) -> sf::RectangleShape {
    sf::RectangleShape r;
    r.setSize({rect.width, rect.height});
    r.setPosition(rect.left, rect.top);
    r.setFillColor({200, 0, 0, 95});
    return r;
}

// Network client for Game
class GameClient : public game::client::ClientNetworkManager {
public:
    void onConnectAck(game::core::Entity::ID entityID) override {
        std::cout << "✅ Connected to server! Entity ID: " << entityID << std::endl;
        myEntityID = entityID;
    }
    
    void onSnapshot(game::network::Packet& packet) override {
        packet.resetRead();
        
        uint32_t entityCount;
        if (!packet.read(entityCount)) {
            return;
        }
        
        // Clear old entities
        remoteEntities.clear();
        
        // Read entities from snapshot
        for (uint32_t i = 0; i < entityCount; ++i) {
            game::core::Entity::ID entityID;
            if (!packet.read(entityID)) break;
            
            float posX, posY;
            if (!packet.read(posX) || !packet.read(posY)) break;
            
            float sizeX, sizeY;
            if (!packet.read(sizeX) || !packet.read(sizeY)) break;
            
            uint8_t r, g, b, a;
            if (!packet.read(r) || !packet.read(g) || !packet.read(b) || !packet.read(a)) break;
            
            RemoteEntity entity;
            entity.position = sf::Vector2f(posX, posY);
            entity.size = sf::Vector2f(sizeX, sizeY);
            entity.color = sf::Color(r, g, b, a);
            
            remoteEntities[entityID] = entity;
        }
    }
    
    void onDisconnect() override {
        std::cout << "Disconnected from server" << std::endl;
        remoteEntities.clear();
        myEntityID = 0;
    }
    
    game::core::Entity::ID myEntityID = 0;
    
    struct RemoteEntity {
        sf::Vector2f position;
        sf::Vector2f size;
        sf::Color color;
    };
    
    std::map<game::core::Entity::ID, RemoteEntity> remoteEntities;
};

struct Game {
    TileMap tilemap;
    sf::RectangleShape player;

    std::vector<sf::FloatRect> colliders;
    bool show_colliders = false;

    sf::View camera;
    sf::FloatRect camera_bounds;
    
    // Network
    GameClient networkClient;
    bool connectedToServer = false;
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 7777;
    sf::Vector2f initialPlayerPosition;  // LDtk'den yüklenen initial pozisyon

    void init(const ldtk::Project& ldtk, bool reloading = false) {
        // get the world from the project
        auto& world = ldtk.getWorld();

        // get first level from the world
        auto& ldtk_level0 = world.getLevel("Level_0");

        // load the TileMap from the level
        TileMap::path = ldtk.getFilePath().directory();
        tilemap.load(ldtk_level0);
        
        // Initialize network client
        if (!reloading) {
            if (networkClient.initialize()) {
                // Connect with initial player position from LDtk
                if (networkClient.connect(serverIp, serverPort, initialPlayerPosition)) {
                    connectedToServer = true;
                    std::cout << "Connecting to server " << serverIp << ":" << serverPort 
                              << " with initial position (" << initialPlayerPosition.x 
                              << ", " << initialPlayerPosition.y << ")..." << std::endl;
                } else {
                    std::cerr << "Failed to connect to server" << std::endl;
                }
            } else {
                std::cerr << "Failed to initialize network client" << std::endl;
            }
        }

        // get Entities layer from level_0
        auto& entities_layer = ldtk_level0.getLayer("Entities");

        // retrieve collider entities from entities layer and store them in the colliders vector
        colliders.clear();
        for (ldtk::Entity& col : entities_layer.getEntitiesByName("Collider")) {
            colliders.emplace_back(
                (float)col.getPosition().x, (float)col.getPosition().y,
                (float)col.getSize().x, (float)col.getSize().y
            );
        }

        // get the Player entity, and its 'color' field
        auto& player_ent = entities_layer.getEntitiesByName("Player")[0].get();
        auto& player_color = player_ent.getField<ldtk::Color>("color").value();

        // initialize player shape
        player.setSize({8, 16});
        player.setOrigin(4, 16);
        if (!reloading) {
            initialPlayerPosition = sf::Vector2f(
                static_cast<float>(player_ent.getPosition().x + 8),
                static_cast<float>(player_ent.getPosition().y + 16)
            );
            player.setPosition(initialPlayerPosition);
        }
        player.setFillColor({player_color.r, player_color.g, player_color.b});

        // create camera view
        camera.setSize({400, 250});
        camera.zoom(0.55f);
        camera.setCenter(player.getPosition());
        camera_bounds.left = 0;
        camera_bounds.top = 0;
        camera_bounds.width = static_cast<float>(ldtk_level0.size.x);
        camera_bounds.height = static_cast<float>(ldtk_level0.size.y);
    }

    void update() {
        // move player with keyboard arrows or WASD
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            player.move(0, -1.5);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            player.move(0, 1.5);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            player.move(-1.5, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            player.move(1.5, 0);

        // do collision checks
        auto player_collider = getPlayerCollider(player);
        for (auto& rect : colliders) {
            sf::FloatRect intersect;
            if (player_collider.intersects(rect, intersect)) {
                if (intersect.width < intersect.height) {
                    if (player_collider.left < intersect.left)
                        player.move(-intersect.width, 0);
                    else
                        player.move(intersect.width, 0);
                }
                else {
                    if (player_collider.top < intersect.top)
                        player.move(0, -intersect.height);
                    else
                        player.move(0, intersect.height);
                }
            }
        }

        // update camera
        camera.move((player.getPosition() - camera.getCenter())/5.f);

        auto cam_size = camera.getSize();

        {
            auto cam_pos = camera.getCenter();
            // check for camera X limit
            if (cam_pos.x - cam_size.x / 2 < camera_bounds.left) {
                camera.setCenter(camera_bounds.left + cam_size.x / 2, cam_pos.y);
            }
            else if (cam_pos.x + cam_size.x / 2 > camera_bounds.left + camera_bounds.width) {
                camera.setCenter(camera_bounds.left + camera_bounds.width - cam_size.x / 2, cam_pos.y);
            }
        }

        {
            auto cam_pos = camera.getCenter();
            // check for camera Y limit
            if (cam_pos.y - cam_size.y / 2 < camera_bounds.top) {
                camera.setCenter(cam_pos.x, camera_bounds.top + cam_size.y / 2);
            }
            else if (cam_pos.y + cam_size.y / 2 > camera_bounds.top + camera_bounds.height) {
                camera.setCenter(cam_pos.x, camera_bounds.top + camera_bounds.height - cam_size.y / 2);
            }
        }
    }

    void render(sf::RenderTarget& target) {
        target.setView(camera);

        // draw map background layers
        target.draw(tilemap.getLayer("Ground"));
        target.draw(tilemap.getLayer("Trees"));
        if (show_colliders) {
            // draw map colliders
            for (auto& rect : colliders)
                target.draw(getColliderShape(rect));
        }

        // Draw all players from server snapshot (including ourselves)
        if (connectedToServer && !networkClient.remoteEntities.empty()) {
            for (const auto& [entityID, remoteEntity] : networkClient.remoteEntities) {
                // If this is our entity, use local player shape but update position
                if (entityID == networkClient.myEntityID && networkClient.myEntityID != 0) {
                    // Server pozisyonu kullan
                    // Server pozisyonu muhtemelen top-left, player origin (4, 16) = bottom-center
                    // Origin offset: (4, 16) = (size.x/2, size.y)
                    sf::Vector2f serverPos = remoteEntity.position;
                    // Server pozisyonu top-left ise, origin'e göre ayarla
                    // Eğer server pozisyonu center ise, direkt kullan
                    // Şimdilik server pozisyonunu direkt kullan (server'da center olarak saklanıyor olabilir)
                    player.setPosition(serverPos);
                    target.draw(player);
                } else {
                    // Draw other players (remote entities)
                    sf::RectangleShape remotePlayer;
                    remotePlayer.setSize(remoteEntity.size);
                    remotePlayer.setPosition(remoteEntity.position);
                    remotePlayer.setFillColor(remoteEntity.color);
                    // Set origin to match local player (bottom-center)
                    remotePlayer.setOrigin(remoteEntity.size.x * 0.5f, remoteEntity.size.y);
                    target.draw(remotePlayer);
                }
            }
        } else {
            // Not connected or no entities yet, draw local player
        target.draw(player);
        }
        if (show_colliders) {
            // draw player collider
            target.draw(getColliderShape(getPlayerCollider(player)));
        }

        // draw map top layer
        target.draw(tilemap.getLayer("Trees_top"));
    }
};

int main() {
    ldtk::Project project;
    std::string ldtk_filename = "assets/maps/world1.ldtk";
    try {
        project.loadFromFile(ldtk_filename);
        std::cout << "LDtk World \"" << project.getFilePath() << "\" was loaded successfully." << std::endl;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    // initialize the game from the LDtk project data
    Game game;
    game.init(project);

    // create the window
    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 500), "LDtkLoader - SFML");
    window.setFramerateLimit(60);

    // Network heartbeat timer
    auto lastHeartbeat = std::chrono::steady_clock::now();
    const float heartbeatInterval = 1.0f;  // 1 second

    // start game loop
    while(window.isOpen()) {

        // EVENTS
        sf::Event event{};
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::F1)
                    game.show_colliders = !game.show_colliders;
                else if (event.key.code == sf::Keyboard::F5) {
                    // reload the LDtk project and reinitialize the game
                    project.loadFromFile(ldtk_filename);
                    game.init(project, true);
                    std::cout << "Reloaded project " << project.getFilePath() << std::endl;
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Process network packets
        if (game.connectedToServer) {
            game.networkClient.processPackets();
            
            // Send heartbeat periodically
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<float>(now - lastHeartbeat).count();
            
            if (elapsed >= heartbeatInterval && game.networkClient.isConnected()) {
                game::network::Packet heartbeat(game::network::PacketType::HEARTBEAT);
                heartbeat.setSequence(1);
                heartbeat.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count()));
                
                game.networkClient.sendPacket(heartbeat);
                lastHeartbeat = now;
            }
        }

        // UPDATE
        game.update();

        // RENDER
        window.clear();
        game.render(window);
        window.display();

    }
    
    // Disconnect from server
    if (game.connectedToServer) {
        game.networkClient.disconnect();
        game.networkClient.shutdown();
    }
    return 0;
}
