#pragma once

#include <cstddef>
#include <vector>

#include "geometry.hpp"
#include "particle.hpp"

// Para cada particula (indexada desde 0), los ids de sus vecinas ordenados.
using NeighborList = std::vector<std::vector<int>>;

std::size_t pairCount(const NeighborList& neighbors);

// Mayor M admitido: exige L/M > rc + 2*rmax (ver README).
int maxCellsPerSide(double side, double interactionRadius, double maxRadius);

NeighborList bruteForceNeighbors(const Particles& particles,
                                 const Domain& domain,
                                 double interactionRadius);

// Lanza std::invalid_argument si M supera el maximo admitido.
NeighborList cellIndexNeighbors(const Particles& particles,
                                const Domain& domain, int cellsPerSide,
                                double interactionRadius);
