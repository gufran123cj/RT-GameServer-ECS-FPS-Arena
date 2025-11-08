#include "../net/Socket.hpp"
#include "../net/Packet.hpp"
#include "../components/InputComponent.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace game;
using namespace game::net;
using namespace game::components;

int main(int argc, char* argv[]) {
    std::cout << "=== Test Client ===" << std::endl;
    
    // Server adresi
    std::string serverIP = "127.0.0.1";
    uint16_t serverPort = 7777;
    
    if (argc > 1) serverIP = argv[1];
    if (argc > 2) serverPort = static_cast<uint16_t>(std::atoi(argv[2]));
    
    // UDP socket oluştur
    UDPSocket socket;
    
    // Socket'i bind et (client için rastgele port)
    Address clientAddress("0.0.0.0", 0);
    if (!socket.bind(clientAddress)) {
        std::cerr << "Failed to bind client socket" << std::endl;
        return 1;
    }
    
    std::cout << "Client socket bound. Connecting to server " 
              << serverIP << ":" << serverPort << std::endl;
    
    // CONNECT paketi oluştur
    Address serverAddress(serverIP, serverPort);
    PacketHeader header;
    header.type = PacketType::CONNECT;
    header.sequence = 0;
    header.serverTick = 0;
    header.playerID = INVALID_PLAYER;
    
    // Paketi gönder
    if (socket.send(serverAddress, &header, sizeof(header))) {
        std::cout << "CONNECT packet sent to server" << std::endl;
    } else {
        std::cerr << "Failed to send CONNECT packet" << std::endl;
        return 1;
    }
    
    // Kısa bir bekleme (server'ın işlemesi için)
    std::cout << "Waiting 500ms for server to process connection..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Response beklemeden devam et (server response göndermiyor şu an)
    // Player ID'yi 0 olarak varsay (ilk player)
    PlayerID playerID = 0;
    std::cout << "Assuming Player ID: " << playerID << " (server will assign on connect)" << std::endl;
    
    // PHASE 2 TEST: Send INPUT packets
    std::cout << "\n=== PHASE 2 TEST: Sending INPUT packets ===" << std::endl;
    std::cout << "Sending input packets every 100ms (10 packets/sec)" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    header.type = PacketType::INPUT;
    header.playerID = playerID;
    uint32_t sequence = 1;
    
    // Simulate different input states
    uint16_t inputFlags = 0;
    float mouseYaw = 0.0f;
    float mousePitch = 0.0f;
    
    while (true) {
        // Simulate input: Forward + Right movement, rotating camera
        inputFlags = INPUT_FORWARD | INPUT_RIGHT;
        mouseYaw += 1.0f; // Rotate camera
        mousePitch = 0.0f;
        
        // Create INPUT packet
        header.sequence = sequence++;
        header.serverTick = 0; // Will be set by server
        
        InputPacket inputPacket;
        inputPacket.flags = inputFlags;
        inputPacket.mouseYaw = mouseYaw;
        inputPacket.mousePitch = mousePitch;
        
        // Send packet: Header + InputPacket
        PacketWriter writer;
        writer.write(header);
        writer.write(inputPacket);
        
        if (socket.send(serverAddress, writer.getData(), writer.getSize())) {
            if (sequence % 10 == 0) { // Print every 10 packets
                std::cout << "[INPUT #" << sequence - 1 << "] flags=" << inputFlags 
                          << " yaw=" << mouseYaw << " pitch=" << mousePitch << std::endl;
            }
        }
        
        // Check for responses
        Packet packet;
        if (socket.receive(packet, 10)) {
            if (packet.size >= sizeof(PacketHeader)) {
                PacketReader reader(packet.data.data(), packet.size);
                PacketHeader recvHeader;
                if (reader.read(recvHeader)) {
                    // Handle response if needed
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 packets/sec
    }
    
    socket.close();
    return 0;
}

