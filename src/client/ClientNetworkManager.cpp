#include "ClientNetworkManager.hpp"
#include "../network/PacketTypes.hpp"
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <chrono>

namespace game::client {

ClientNetworkManager::ClientNetworkManager() 
    : connected(false)
    , entityID(0)
    , nextSequenceNumber(1) {
}

ClientNetworkManager::~ClientNetworkManager() {
    shutdown();
}

bool ClientNetworkManager::initialize() {
    // Bind to any available port (0 = auto-assign)
    if (socket.bind(0) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind client socket" << std::endl;
        return false;
    }
    
    socket.setBlocking(false);  // Non-blocking mode
    
    unsigned short localPort = socket.getLocalPort();
    std::cout << "Client socket bound to port " << localPort << std::endl;
    
    return true;
}

void ClientNetworkManager::shutdown() {
    if (connected) {
        disconnect();
    }
    socket.unbind();
}

bool ClientNetworkManager::connect(const std::string& serverIp, uint16_t serverPort, const sf::Vector2f& initialPosition) {
    if (connected) {
        std::cerr << "Already connected to server" << std::endl;
        return false;
    }
    
    serverAddress = game::network::Address(serverIp, serverPort);
    
    // Send CONNECT packet with initial position
    game::network::Packet packet(game::network::PacketType::CONNECT);
    packet.setSequence(nextSequenceNumber++);
    packet.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count()));
    
    // Send initial position to server
    packet.write(initialPosition.x);
    packet.write(initialPosition.y);
    
    if (!sendPacket(packet)) {
        std::cerr << "Failed to send CONNECT packet" << std::endl;
        return false;
    }
    
    std::cout << "Connecting to server " << serverAddress.toString() << "..." << std::endl;
    
    // Wait for CONNECT_ACK (non-blocking, will be processed in processPackets)
    connected = false;  // Will be set to true when we receive CONNECT_ACK
    
    return true;
}

void ClientNetworkManager::disconnect() {
    if (!connected) {
        return;
    }
    
    // Send DISCONNECT packet
    game::network::Packet packet(game::network::PacketType::DISCONNECT);
    packet.setSequence(nextSequenceNumber++);
    packet.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count()));
    
    sendPacket(packet);
    
    connected = false;
    entityID = 0;
    
    std::cout << "Disconnected from server" << std::endl;
}

int ClientNetworkManager::processPackets() {
    int packetCount = 0;
    
    // Temporary buffer for receiving
    static uint8_t receiveBuffer[MAX_PACKET_SIZE];
    
    while (true) {
        std::size_t received = 0;
        sf::IpAddress senderIp;
        unsigned short senderPort;
        
        sf::Socket::Status status = socket.receive(
            receiveBuffer,
            MAX_PACKET_SIZE,
            received,
            senderIp,
            senderPort
        );
        
        if (status == sf::Socket::Status::NotReady || status == sf::Socket::Status::Disconnected) {
            break;  // No more packets
        }
        
        if (status == sf::Socket::Status::Done && received > 0) {
            // Create packet from received data
            game::network::Packet packet;
            packet.setData(receiveBuffer, received);
            
            handlePacket(packet);
            packetCount++;
        }
    }
    
    return packetCount;
}

bool ClientNetworkManager::sendPacket(const game::network::Packet& packet) {
    sf::Socket::Status status = socket.send(
        packet.getData(),
        static_cast<std::size_t>(packet.getSize()),
        serverAddress.getIpAddress(),
        serverAddress.getPort()
    );
    
    return status == sf::Socket::Status::Done;
}

void ClientNetworkManager::handlePacket(const game::network::Packet& packet) {
    game::network::PacketType type = packet.getType();
    
    switch (type) {
        case game::network::PacketType::CONNECT_ACK: {
            game::network::Packet& nonConstPacket = const_cast<game::network::Packet&>(packet);
            nonConstPacket.resetRead();
            game::core::Entity::ID receivedEntityID;
            if (nonConstPacket.read(receivedEntityID)) {
                entityID = receivedEntityID;
                connected = true;
                std::cout << "✅ Connected to server! Entity ID: " << entityID << std::endl;
                onConnectAck(entityID);
            }
            break;
        }
        
        case game::network::PacketType::SNAPSHOT: {
            // Pass non-const reference for reading
            game::network::Packet& nonConstPacket = const_cast<game::network::Packet&>(packet);
            onSnapshot(nonConstPacket);
            break;
        }
        
        case game::network::PacketType::DISCONNECT: {
            std::cout << "Server disconnected" << std::endl;
            connected = false;
            onDisconnect();
            break;
        }
        
        default:
            break;
    }
}

} // namespace game::client

