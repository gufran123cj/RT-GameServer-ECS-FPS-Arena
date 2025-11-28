#include "GameController.hpp"
#include <iostream>
#include <chrono>

namespace game::client {

void GameController::update(GameModel& model, const sf::Window& window) {
    // Update player position from server snapshot
    updatePlayerPosition(model);
    
    // Handle input and send to server (only if window has focus)
    handleInput(model, window);
}

void GameController::processNetwork(GameModel& model) {
    if (model.connectedToServer) {
        model.networkClient.processPackets();
    }
}

void GameController::updatePlayerPosition(GameModel& model) {
    if (!model.connectedToServer) {
        return;
    }
    
    if (model.networkClient.myEntityID == game::INVALID_ENTITY) {
        return;
    }
    
    auto it = model.networkClient.remoteEntities.find(model.networkClient.myEntityID);
    if (it == model.networkClient.remoteEntities.end()) {
        return;
    }
    
    sf::Vector2f serverPos = it->second.position;
    
    sf::Vector2f oldPos = model.player.getPosition();
    model.player.setPosition(serverPos);
    
    bool hasCollision = PlayerCollision::checkCollision(model.player, model.colliders);
    
    if (hasCollision) {
        model.serverPositionInvalid = true;
        if (model.hasLastValidPosition) {
            model.player.setPosition(model.lastValidPosition);
        } else {
            model.player.setPosition(oldPos);
            model.lastValidPosition = oldPos;
            model.hasLastValidPosition = true;
        }
    } else {
        model.lastValidPosition = serverPos;
        model.hasLastValidPosition = true;
        model.serverPositionInvalid = false;
    }
}

void GameController::handleInput(GameModel& model, const sf::Window& window) {
    // CRITICAL: Only process input if this window has focus
    // This prevents all clients from responding to the same keyboard input
    if (!window.hasFocus()) {
        return;  // This window is not active, don't process input
    }
    
    float velX = 0.0f, velY = 0.0f;
    const float moveSpeed = 60.0f;
    const float deltaTime = 1.0f / 60.0f;
    
    bool wPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool sPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool aPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool dPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
    bool leftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    
    // Log key presses (only when a key is actually pressed)
    static bool lastW = false, lastS = false, lastA = false, lastD = false;
    static bool lastUp = false, lastDown = false, lastLeft = false, lastRight = false;
    
    if ((wPressed && !lastW) || (aPressed && !lastA) || (sPressed && !lastS) || (dPressed && !lastD) ||
        (upPressed && !lastUp) || (downPressed && !lastDown) || (leftPressed && !lastLeft) || (rightPressed && !lastRight)) {
        std::cout << "Input from playerId: " << model.networkClient.myEntityID << std::endl;
    }
    
    lastW = wPressed;
    lastS = sPressed;
    lastA = aPressed;
    lastD = dPressed;
    lastUp = upPressed;
    lastDown = downPressed;
    lastLeft = leftPressed;
    lastRight = rightPressed;
    
    if (upPressed || wPressed)
        velY = -moveSpeed;
    if (downPressed || sPressed)
        velY = moveSpeed;
    if (leftPressed || aPressed)
        velX = -moveSpeed;
    if (rightPressed || dPressed)
        velX = moveSpeed;
    
    // Collider kontrolü: Eğer INPUT yönü collider'a doğruysa, velocity'yi sıfırla
    if (model.connectedToServer && model.networkClient.isConnected() && 
        !model.serverPositionInvalid && (velX != 0 || velY != 0)) {
        if (wouldCollide(model, velX, velY)) {
            velX = 0.0f;
            velY = 0.0f;
        }
    }
    
    // EK KONTROL: Server pozisyonu geçersizse INPUT paketlerini hiç gönderme
    if (model.serverPositionInvalid) {
        velX = 0.0f;
        velY = 0.0f;
    }
    
    // CRITICAL: Only send INPUT if we have a valid entity ID assigned by the server
    // Each client should ONLY control its own entity, not others
    // NOTE: Entity ID can be 0 (first client), so we only check for INVALID_ENTITY
    if (model.connectedToServer && 
        model.networkClient.isConnected() && 
        !model.serverPositionInvalid &&
        model.networkClient.myEntityID != game::INVALID_ENTITY) {
        
        game::network::Packet inputPacket(game::network::PacketType::INPUT);
        inputPacket.setSequence(1);
        inputPacket.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()));
        inputPacket.write(velX);
        inputPacket.write(velY);
        
        model.networkClient.sendPacket(inputPacket);
    } else {
        // Debug: Log why INPUT is not being sent
        static int debugCounter = 0;
        if (debugCounter++ % 60 == 0 && (velX != 0 || velY != 0)) {
            std::cout << "[CLIENT] NOT sending INPUT - connected: " << model.connectedToServer
                      << ", isConnected: " << model.networkClient.isConnected()
                      << ", serverPositionInvalid: " << model.serverPositionInvalid
                      << ", myEntityID: " << model.networkClient.myEntityID << std::endl;
        }
    }
}

bool GameController::wouldCollide(const GameModel& model, float velX, float velY) {
    const float moveSpeed = 60.0f;
    const float deltaTime = 1.0f / 60.0f;
    
    // Mevcut pozisyonu al
    sf::Vector2f currentPos = model.player.getPosition();
    
    // Bir sonraki pozisyonu hesapla (daha büyük adım - birkaç frame ilerisi)
    const float checkDistance = moveSpeed * deltaTime * 2.0f;  // 2 frame ilerisi
    sf::Vector2f nextPos = currentPos + sf::Vector2f(
        velX != 0 ? (velX > 0 ? checkDistance : -checkDistance) : 0.0f,
        velY != 0 ? (velY > 0 ? checkDistance : -checkDistance) : 0.0f
    );
    
    // Check if would collide at next position
    return PlayerCollision::wouldCollideAt(nextPos, {8.0f, 16.0f}, model.colliders);
}

} // namespace game::client

