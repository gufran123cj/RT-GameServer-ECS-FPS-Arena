#include "../net/Socket.hpp"
#include "../net/Packet.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace game;
using namespace game::net;

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
    
    // Yanıt bekle (5 saniye timeout)
    std::cout << "Waiting for server response..." << std::endl;
    Packet response;
    bool received = false;
    
    for (int i = 0; i < 50; ++i) { // 50 * 100ms = 5 saniye
        if (socket.receive(response, 100)) {
            if (response.size >= sizeof(PacketHeader)) {
                PacketReader reader(response.data.data(), response.size);
                PacketHeader responseHeader;
                if (reader.read(responseHeader)) {
                    std::cout << "Received packet from server:" << std::endl;
                    std::cout << "  Type: " << static_cast<int>(responseHeader.type) << std::endl;
                    std::cout << "  Player ID: " << responseHeader.playerID << std::endl;
                    std::cout << "  Server Tick: " << responseHeader.serverTick << std::endl;
                    received = true;
                    break;
                }
            }
        }
    }
    
    if (!received) {
        std::cout << "No response from server (this is normal - server may not send response yet)" << std::endl;
    }
    
    // Heartbeat gönder (test için)
    std::cout << "\nSending heartbeat packets every 2 seconds..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    header.type = PacketType::HEARTBEAT;
    uint32_t sequence = 1;
    
    while (true) {
        header.sequence = sequence++;
        if (socket.send(serverAddress, &header, sizeof(header))) {
            std::cout << "Heartbeat #" << sequence - 1 << " sent" << std::endl;
        }
        
        // Yanıt kontrol et
        Packet packet;
        if (socket.receive(packet, 100)) {
            if (packet.size >= sizeof(PacketHeader)) {
                PacketReader reader(packet.data.data(), packet.size);
                PacketHeader recvHeader;
                if (reader.read(recvHeader)) {
                    std::cout << "  -> Received response: Type=" 
                              << static_cast<int>(recvHeader.type) << std::endl;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    socket.close();
    return 0;
}

