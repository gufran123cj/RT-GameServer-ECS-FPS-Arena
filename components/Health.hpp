#pragma once

#include "../ecs/Component.hpp"

namespace game::components {

// Health component - stores entity health and max health
class Health : public ecs::ComponentBase<Health> {
public:
    float current;
    float maximum;
    bool isAlive;

    Health() : current(100.0f), maximum(100.0f), isAlive(true) {}
    Health(float maxHealth) : current(maxHealth), maximum(maxHealth), isAlive(true) {}

    void takeDamage(float damage) {
        current -= damage;
        if (current <= 0.0f) {
            current = 0.0f;
            isAlive = false;
        }
    }

    void heal(float amount) {
        if (!isAlive) return;
        current += amount;
        if (current > maximum) {
            current = maximum;
        }
    }

    float getHealthPercentage() const {
        return maximum > 0.0f ? (current / maximum) : 0.0f;
    }

    std::unique_ptr<ecs::Component> clone() const override {
        auto health = std::make_unique<Health>(maximum);
        health->current = current;
        health->isAlive = isAlive;
        return health;
    }
};

} // namespace game::components

