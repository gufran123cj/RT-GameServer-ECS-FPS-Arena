# Real-Time Game Server + ECS (FPS-lite Arena)

A real-time authoritative game server built with C++17, featuring an ECS (Entity Component System) architecture. Provides a foundation for FPS-style or arena-type games.

![Multiplayer Game Screen](images/mp_screen.png)

*Top-down 2D multiplayer game view with Raylib client*

## 🎮 Features

- ✅ **ECS (Entity-Component-System)** architecture - Flexible and scalable game logic
- ✅ **Network layer** - UDP socket abstraction (Windows/Linux)
- ✅ **Room-based** server - Multiple game room support
- ✅ **60/120 tick** server loop - High-performance real-time simulation
- ✅ **Physics** layer - BVH spatial partitioning with collision detection
- ✅ **Matchmaking** - Simple queue-based player matching system
- ✅ **Anti-cheat-lite** - Basic cheat prevention controls (packet rate limiting, movement validation)
- ✅ **Snapshot** serialization - Component-based snapshot system
- ✅ **2D Game Client** - Raylib-based top-down 2D visualization
- ✅ **150x150 Map** - Large game world with static obstacles and walls
- ✅ **Multiplayer Support** - Multiple players can connect and play simultaneously
- ✅ **Random Spawn System** - Players spawn at random locations with collision prevention
- ⏳ Delta compression (TODO)
- ⏳ Lag compensation (TODO)
- ⏳ Rollback/rewind (TODO)
- ⏳ Deterministic simulation (TODO)

## 📁 Project Structure

```
Real-Time Game Server + ECS (FPS-lite  Arena)/
├── include/
│   └── common/
│       └── types.hpp          # Basic type definitions
├── ecs/
│   ├── Entity.hpp             # Entity class
│   ├── Component.hpp/cpp      # Component base and registry
│   ├── System.hpp             # System base (template)
│   └── World.hpp/cpp          # ECS World
├── net/
│   ├── Socket.hpp/cpp         # UDP socket wrapper
│   ├── Packet.hpp             # Packet reader/writer
│   └── Snapshot.hpp/cpp       # Snapshot and delta compression
├── physics/
│   └── Physics.hpp/cpp        # Vec3, AABB, BVH
├── matchmaker/
│   └── Matchmaker.hpp/cpp     # Game matching
├── anti-cheat-lite/
│   └── AntiCheat.hpp/cpp      # Basic anti-cheat controls
├── src/
│   ├── Server.hpp/cpp         # Main server class
│   ├── main.cpp               # Server entry point
│   ├── GameClient.cpp         # Raylib 2D game client
│   ├── TestClient.cpp         # Test client
│   └── MiniGameViewer.cpp     # ASCII map viewer
├── components/
│   ├── Position.hpp           # Position component
│   ├── Velocity.hpp           # Velocity component
│   ├── Health.hpp             # Health component
│   ├── PlayerComponent.hpp    # Player identification
│   ├── InputComponent.hpp     # Player input
│   ├── Transform.hpp          # Transform component
│   └── CollisionComponent.hpp # Collision bounds
├── systems/
│   ├── MovementSystem.hpp     # Player movement system
│   └── PhysicsSystem.hpp      # Physics and collision system
├── build.bat                  # Server build script
├── build-client.bat           # Client build script
├── build-gameclient.bat       # Game client build script
├── start-server.bat           # Server starter with IP display
├── get-server-ip.bat          # IP address finder
├── NETWORK_SETUP.md           # Network configuration guide
└── README.md
```

## 🛠️ Installation and Building

### Requirements

- **Windows 10/11**
- **MinGW** (Minimalist GNU for Windows) - Must be installed in `D:\MinGW\bin` directory
- **C++17** compatible compiler (g++ 7.0+)

### MinGW Installation

Ensure MinGW is installed in the `D:\MinGW` directory. If it's installed in a different location, update the `MINGW_PATH` variable in `build.bat` and `build-client.bat` files.

### Building the Server

1. Navigate to the project directory:
```bash
cd "D:\Real-Time Game Server + ECS (FPS-lite  Arena)"
```

2. Run `build.bat` to build the server:
```bash
.\build.bat
```

This will create the `GameServer.exe` executable.

### Building the Game Client (Raylib 2D Client)

1. Navigate to the project directory:
```bash
cd "D:\Real-Time Game Server + ECS (FPS-lite  Arena)"
```

2. Run `build-gameclient.bat` to build the 2D game client:
```bash
.\build-gameclient.bat
```

This will create the `GameClient.exe` executable.

**Note:** Raylib library must be present in the `raylib/` directory. See `RAYLIB_SETUP.md` for setup instructions.

### Building the Test Client

1. Navigate to the project directory:
```bash
cd "D:\Real-Time Game Server + ECS (FPS-lite  Arena)"
```

2. Run `build-client.bat` to build the test client:
```bash
.\build-client.bat
```

This will create the `TestClient.exe` executable.

## 🚀 Usage

### Running the Server

To start the server, run the `GameServer.exe` file:

```bash
# Default: port 7777, 60 tick
GameServer.exe

# Custom port and tick rate
GameServer.exe 7777 60
```

When the server starts successfully, you will see the following output:
```
=== Game Server (FPS-lite / Arena) ===
C++17 | ECS | Authoritative Server
=====================================
Game Server initialized on 0.0.0.0:7777 (Tick Rate: 60)
Server running. Press Ctrl+C to stop.
```

### Running the Game Client (2D Visualization)

While the server is running, open a **new terminal window** and run the game client:

```bash
# Default: 127.0.0.1:7777
GameClient.exe

# For a different server address and port
GameClient.exe 192.168.1.100 7777
```

The game client features:
- **Top-down 2D view** - Raylib-based visualization
- **WASD movement** - Direct keyboard controls
- **Mouse look** - Mouse controls for direction
- **Zoom controls** - Mouse wheel or +/- keys
- **Real-time updates** - Live snapshot rendering
- **Multiplayer view** - See all players and map objects

### Running the Test Client

For basic connection testing:

```bash
# Default: 127.0.0.1:7777
TestClient.exe

# For a different server address and port
TestClient.exe 127.0.0.1 7777
```

### Multiplayer Setup

To play with friends on the same network:

1. **Start the server:**
   ```bash
   start-server.bat
   ```
   This will display your server's IP address.

2. **On client computers:**
   ```bash
   GameClient.exe [SERVER_IP] 7777
   ```
   Replace `[SERVER_IP]` with the IP address shown by the server.

3. **Firewall Configuration:**
   - Windows Firewall must allow UDP port 7777
   - See `NETWORK_SETUP.md` for detailed instructions

### Connection Test

You should see the following message in the server terminal:
```
Player 0 connected from 127.0.0.1:XXXXX
Player 1 connected from 192.168.1.101:XXXXX
```

This indicates that clients have successfully connected.

## 🔧 Technical Details

### ECS Architecture

- **Entity**: ID + component mask
- **Component**: Type-safe component registry
- **System**: Template-based system with component filtering
- **World**: Entity/component/system management

### Network

- UDP socket abstraction (Windows/Linux compatible)
- Packet header: type, sequence, tick, playerID
- Snapshot history (64 snapshots)
- Infrastructure ready for delta compression (implementation TODO)

### Physics

- BVH (Bounding Volume Hierarchy) spatial partitioning
- AABB collision detection
- Vec3 math library

### Matchmaking

- Simple queue-based matching (no rating system)
- Automatic room creation when 2 players are matched
- Players can request matches and cancel queue
- FIND_MATCH and MATCH_FOUND packet types

### Anti-Cheat

- Packet rate limiting
- Movement speed validation
- Suspicious action tracking

## 📊 Development Status

**✅ Completed:**
- Basic architecture
- ECS framework
- Network layer
- Server framework
- Physics system (BVH, collision detection)
- Component serialization
- Snapshot system
- 2D Game Client (Raylib)
- Matchmaking system
- Random spawn system
- Map objects (walls/obstacles)
- 150x150 game world
- Multiplayer support

**⏳ In Progress:**
- Delta compression implementation
- Lag compensation
- Rollback/rewind

**📋 Planned:**
- Deterministic simulation
- Rating-based matchmaking
- Lua/AngelScript scripting
- Glicko-2 rating system
- Profiling tools
- Shooting mechanics
- Health system improvements

## ⚠️ Important Notes

- **C++ Standard**: The project uses C++17 standard (due to MinGW 13.2.0 C++20 incompatibility)
- **MinGW Version**: Tested with MinGW 13.2.0
- **Platform**: Currently optimized for Windows, Linux support is planned
- **Raylib**: Required for GameClient. Must be in `raylib/` directory
- **Network**: Server binds to `0.0.0.0` (all interfaces). Use `get-server-ip.bat` to find your IP
- **Firewall**: UDP port 7777 must be open for multiplayer connections
- **Production Use**: This project is in active development. Additional tests and optimizations are required for production use

## 🌐 Network Configuration

For detailed network setup instructions, see `NETWORK_SETUP.md`.

Quick start:
1. Run `start-server.bat` to start the server and see your IP
2. On client machines: `GameClient.exe [SERVER_IP] 7777`
3. Ensure Windows Firewall allows UDP port 7777

## 🤝 Contributing

Contributions are welcome! Before submitting a pull request, please:
1. Ensure your code is clean and readable
2. Follow the existing code style
3. Test the connection with the test client

## 📧 Contact

Feel free to open an issue for questions or suggestions.
