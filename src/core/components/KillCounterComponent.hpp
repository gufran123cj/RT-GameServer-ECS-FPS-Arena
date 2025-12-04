#pragma once
#include "../Component.hpp"

namespace game::core::components {
struct KillCounterComponent {
    int killCount;

    KillCounterComponent() : killCount(0) {}
    KillCounterComponent(int kills) : killCount(kills) {}

    void addKill() { killCount++; }
    int getKills() const { return killCount; }
};
} // namespace game::core::components

