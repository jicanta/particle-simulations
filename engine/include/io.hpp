#pragma once

#include <string>

#include "neighbor_list.hpp"
#include "particle.hpp"

struct SystemState {
    Particles particles;
    double side = 0.0;
    double time = 0.0;
};

SystemState readSystem(const std::string& staticPath, const std::string& dynamicPath);
void writeSystem(const std::string& staticPath, const std::string& dynamicPath, const SystemState& state);
void writeNeighbors(const std::string& path, const NeighborList& neighbors);
