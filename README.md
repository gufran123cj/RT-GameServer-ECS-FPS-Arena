# RT-GameServer-ECS-FPS-Arena

Top-down multiplayer game project - Game server and client application built with ECS (Entity-Component-System) architecture, featuring LDtk map loading, server-side authoritative collision detection, and SFML rendering support.

## 🎮 Project Features

### ✅ Completed Features

- **ECS Architecture**: Modular game logic with Entity-Component-System pattern
  - Component-based entity management
  - System-based game logic
  - Type-safe component storage (SparseSet)
  
- **Multiplayer Server**: Authoritative game server
  - UDP network protocol
  - Client connection management
  - Entity spawning and snapshot system
  - Fixed timestep game loop (60 TPS)

- **Client Integration**: LDtk-SFML game integration
  - Automatic connection to server and entity synchronization
  - Remote player rendering
  - Local player movement (WASD + Arrow keys)

- **LDtk Map System**: Level Designer Toolkit integration
  - Tile layer rendering
  - Entity layer loading
  - IntGrid collision layer loading
  - Hot reload (F5)

- **Collision Detection**: Authoritative server-side collision system
  - Automatic collision loading from IntGrid "Collisions" layer
  - Client-side collision prediction
  - Server-side authoritative collision detection
  - CollisionSystem (ECS) integration

- **SFML Rendering**: 2D graphics system
  - High-performance tile rendering with VertexArray
  - Sprite component system
  - Camera following
  - Collision debug visualization (F1)

## 🏗️ Project Structure

```
RT-GameServer-ECS-FPS-Arena/
├── src/
│   ├── core/                    # ECS Core Framework
│   │   ├── Entity.hpp           # Entity ID and generation system
│   │   ├── Component.hpp         # Component type traits
│   │   ├── ComponentStorage.hpp # SparseSet-based storage
│   │   ├── ComponentRegistry.hpp# Component type registry
│   │   ├── System.hpp           # System base class
│   │   ├── SystemManager.hpp    # System lifecycle management
│   │   ├── World.hpp/cpp        # ECS World (central management)
│   │   ├── components/          # Game components
│   │   │   ├── PositionComponent.hpp
│   │   │   ├── VelocityComponent.hpp
│   │   │   └── SpriteComponent.hpp
│   │   └── systems/             # Game systems
│   │       └── MovementSystem.hpp
│   ├── game/                    # Game MVC Components
│   │   ├── GameModel.hpp/cpp    # Game data and state
│   │   ├── GameView.hpp/cpp     # Rendering logic
│   │   ├── GameController.hpp/cpp # Input and game logic
│   │   ├── GameClient.hpp/cpp   # Network client wrapper
│   │   ├── PlayerCollision.hpp/cpp # Client-side collision
│   │   └── GameConstants.hpp   # Game constants
│   ├── server/                  # Game Server
│   │   ├── GameServer.hpp/cpp   # Main server logic
│   │   ├── ServerNetworkManager.hpp/cpp  # UDP network management
│   │   ├── ServerConfig.hpp     # Server configuration
│   │   ├── CollisionHelper.hpp/cpp # Server collision utilities
│   │   ├── systems/             # Server systems
│   │   │   └── CollisionSystem.hpp/cpp # Authoritative collision
│   │   └── main.cpp             # Server entry point
│   ├── client/                  # Game Client
│   │   ├── ClientNetworkManager.hpp/cpp  # Client network management
│   │   └── main.cpp             # Test client
│   ├── network/                 # Network Utilities
│   │   ├── Address.hpp          # IP/Port wrapper
│   │   ├── Packet.hpp           # Binary serialization
│   │   └── PacketTypes.hpp      # Packet type enum
│   ├── main.cpp                 # LDtkSFMLGame (main game)
│   ├── TileMap.hpp/cpp          # LDtk map rendering
│   └── test_ecs.cpp             # ECS test executable
├── include/
│   └── common/
│       └── types.hpp             # Common type definitions
├── CMakeLists.txt               # Build configuration
└── README.md                    # This file
```

## 📋 Requirements

### System Requirements
- **Operating System**: Windows 11 (PowerShell)
- **C++ Compiler**: MSVC (Visual Studio 2019+)
- **CMake**: 3.15+

### Dependencies
- **[SFML](https://github.com/SFML/SFML)** 2.6.0: Graphics and network library
  - `sfml-graphics`
  - `sfml-network`
  - `sfml-system`
  - `sfml-window`
  
- **[LDtkLoader](https://github.com/Madour/LDtkLoader)**: LDtk file loading
  - Automatically downloaded via FetchContent in CMake

## 🔨 Build Instructions

### 1. Clone the Project
```powershell
git clone <repository-url>
cd RT-GameServer-ECS-FPS-Arena
```

### 2. Create Build Directory
```powershell
mkdir build
cd build
```

### 3. CMake Configuration
```powershell
cmake ..
```

### 4. Build the Project
```powershell
# Build all projects
cmake --build . --config Release

# Or specific target
cmake --build . --config Release --target gameserver
cmake --build . --config Release --target LDtkSFMLGame
cmake --build . --config Release --target testclient
```

### 5. Run
```powershell
# Start server
.\bin\Release\gameserver.exe

# Start client (separate terminal)
.\bin\Release\LDtkSFMLGame.exe
```

## 🎮 Usage

### Screenshots

![Multiplayer Game Screen](images/gamescreenshot.png)

*Multiplayer game view with two clients connected simultaneously - New map.ldtk integration with active collision detection*

### Starting the Server
```powershell
cd build\bin\Release
.\gameserver.exe
```

The server listens on `127.0.0.1:7777` by default. Log output:
```
GameServer initialized:
  Port: 7777
  Tick Rate: 60 Hz
  Snapshot Rate: 20 Hz
  Max Players: 128
Server: Loaded 200 collision cells from IntGrid layer
Server: Total colliders loaded: 200
GameServer running...
```

### Client Connection
When `LDtkSFMLGame.exe` is run, it automatically connects to the server and spawns the player character on the server.

**Controls:**
- **WASD** or **Arrow Keys**: Character movement
- **F1**: Collision debug view (show/hide colliders)
- **F5**: LDtk world reload
- **ESC**: Close game

### Multiplayer Test
You can test with multiple clients using the `test_multiplayer.ps1` script:
```powershell
.\test_multiplayer.ps1
```

This script:
1. Starts the server
2. Waits 2 seconds
3. Starts the first client
4. Waits 2 seconds
5. Starts the second client

## 🏛️ Architecture Details

### ECS (Entity-Component-System)

**Entity**: Lightweight structure containing only ID and generation
```cpp
struct Entity {
    EntityID id;
    uint32_t generation;
};
```

**Component**: Data structures (Position, Velocity, Sprite)
```cpp
struct PositionComponent {
    sf::Vector2f position;
};
```

**System**: Game logic (MovementSystem, RenderSystem)
```cpp
class MovementSystem : public System {
    void update(float deltaTime) override;
};
```

**World**: Central ECS management
```cpp
World world;
auto entity = world.createEntity();
world.addComponent<PositionComponent>(entity, {100, 200});
```

### Network Protocol

**Packet Types:**
- `CONNECT`: Client connection request to server
- `CONNECT_ACK`: Server connection acknowledgment
- `INPUT`: Client input (velocity)
- `SNAPSHOT`: Game state from server to client
- `HEARTBEAT`: Keep-alive packets
- `DISCONNECT`: Connection termination

**Snapshot System:**
- Server serializes game state every tick (60 TPS)
- Client receives snapshots and renders remote entities
- Client prediction implemented for smooth movement

### Server Authority

Server controls all game state:
- Entity spawning
- Position updates
- **Collision detection** (authoritative)
- Movement validation

Client:
- Sends input (WASD/Arrow keys)
- Receives snapshots and renders
- Client-side collision prediction (for smooth movement)

### Collision System

**Server-Side (Authoritative):**
- Loads collisions from LDtk IntGrid "Collisions" layer
- CollisionSystem runs before MovementSystem
- Prevents entities from moving into colliders
- Validates all movement

**Client-Side (Prediction):**
- Loads same collision data from LDtk
- Predicts collisions before sending input
- Prevents sending input that would cause collision
- Handles server position corrections

## 🚧 Development Status

### MVP (Minimum Viable Product) - In Progress
- [x] ECS Core framework
- [x] Basic server-client connection
- [x] Entity spawning
- [x] Snapshot system
- [x] LDtk integration
- [x] Input handling (client → server)
- [x] Collision system (server-side authoritative)
- [x] Player movement synchronization
- [x] IntGrid collision layer loading
- [ ] Lag compensation
- [ ] Client prediction refinement

### Future Features
- [ ] Client prediction refinement
- [ ] Interpolation
- [ ] Entity replication
- [ ] Game mechanics (shooting, health, etc.)
- [ ] Performance optimization
- [ ] Multiple level support

## 🐛 Known Issues

- Collision detection sometimes triggers too early (under improvement)
- Player may occasionally get stuck when entering collider (under fix)
- Lag compensation not implemented

## 📝 Notes

- Project uses C++17 standard
- SFML DLLs are automatically copied to build directory
- Server and client use the same network protocol
- LDtk map files must be in `assets/maps/` directory
- Tileset files must be in `assets/tilesets/` directory
- Collisions are automatically loaded from IntGrid "Collisions" layer
- Player position is logged to console every 5 seconds (for debugging)
- All collision positions are logged once at startup

## 📄 License

This project is freely usable. Derived from the original LDtk-SFML-Game project.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

**Developer Note**: This project is under active development. API changes may occur until MVP is completed.
