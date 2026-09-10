#pragma once

#include <cstdint>

using EntityID = std::uint32_t;

struct Entity {
    EntityID id;
    std::uint32_t generation;

    bool operator==(const Entity& other) const {
        return id == other.id && generation == other.generation;
    }
};