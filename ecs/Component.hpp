#pragma once

#include "../include/common/types.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>

// Forward declarations
namespace game::net {
    class PacketWriter;
    class PacketReader;
}

namespace game::ecs {

class Component {
public:
    virtual ~Component() = default;
    virtual ComponentTypeID getTypeID() const = 0;
    virtual std::unique_ptr<Component> clone() const = 0;
    
    // Serialization interface (Phase 4)
    virtual bool serialize(net::PacketWriter& writer) const = 0;
    virtual bool deserialize(net::PacketReader& reader) = 0;
    virtual size_t getSerializedSize() const = 0;
};

// Component type registry
class ComponentRegistry {
private:
    static ComponentTypeID nextTypeID;
    static std::unordered_map<std::type_index, ComponentTypeID> typeMap;

public:
    template<typename T>
    static ComponentTypeID getTypeID() {
        static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
        std::type_index typeIndex(typeid(T));
        
        auto it = typeMap.find(typeIndex);
        if (it != typeMap.end()) {
            return it->second;
        }
        
        ComponentTypeID id = nextTypeID++;
        typeMap[typeIndex] = id;
        return id;
    }
};

// Base component class with type ID
template<typename T>
class ComponentBase : public Component {
public:
    static ComponentTypeID getStaticTypeID() {
        static ComponentTypeID typeID = ComponentRegistry::getTypeID<T>();
        return typeID;
    }
    
    ComponentTypeID getTypeID() const override {
        return getStaticTypeID();
    }
};

} // namespace game::ecs

