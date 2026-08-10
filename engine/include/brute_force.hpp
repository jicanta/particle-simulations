#pragma once

#include "geometry.hpp"
#include "neighbor_list.hpp"
#include "particle.hpp"

NeighborList bruteForceNeighbors(const Particles& particles, const Domain& domain, double interactionRadius);
