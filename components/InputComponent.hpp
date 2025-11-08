#pragma once

#include "../ecs/Component.hpp"
#include "../include/common/types.hpp"
#include <cstdint>

namespace game::components {

// Input flags - bit flags for efficient packing
enum InputFlags : uint16_t {
    INPUT_NONE = 0,
    INPUT_FORWARD = 1 << 0,   // W key
    INPUT_BACKWARD = 1 << 1,  // S key
    INPUT_LEFT = 1 << 2,      // A key
    INPUT_RIGHT = 1 << 3,     // D key
    INPUT_JUMP = 1 << 4,      // Space
    INPUT_CROUCH = 1 << 5,    // Ctrl
    INPUT_SPRINT = 1 << 6,    // Shift
    INPUT_SHOOT = 1 << 7,     // Mouse left
    INPUT_AIM = 1 << 8        // Mouse right
};

// Input component - stores player input state
class InputComponent : public ecs::ComponentBase<InputComponent> {
public:
    uint16_t flags;           // Input flags (bitmask)
    float mouseYaw;           // Mouse horizontal rotation (degrees)
    float mousePitch;         // Mouse vertical rotation (degrees)
    SequenceNumber sequence;  // Input sequence number for validation
    Tick inputTick;           // Tick when input was received
    
    InputComponent() 
        : flags(INPUT_NONE)
        , mouseYaw(0.0f)
        , mousePitch(0.0f)
        , sequence(0)
        , inputTick(0) {}
    
    // Helper methods
    bool isPressed(InputFlags flag) const {
        return (flags & flag) != 0;
    }
    
    void setFlag(InputFlags flag, bool pressed) {
        if (pressed) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }
    
    void clear() {
        flags = INPUT_NONE;
        mouseYaw = 0.0f;
        mousePitch = 0.0f;
    }
    
    std::unique_ptr<ecs::Component> clone() const override {
        auto input = std::make_unique<InputComponent>();
        input->flags = flags;
        input->mouseYaw = mouseYaw;
        input->mousePitch = mousePitch;
        input->sequence = sequence;
        input->inputTick = inputTick;
        return input;
    }
};

} // namespace game::components

