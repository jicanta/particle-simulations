#pragma once

#include <vector>

#include "geometry.hpp"
#include "neighbor_list.hpp"
#include "particle.hpp"

class CellIndexMethod {
public:
    static int maxCellsPerSide(double side, double interactionRadius, double maxRadius);

    CellIndexMethod(const Domain& domain, int cellsPerSide, double interactionRadius);

    NeighborList findNeighbors(const Particles& particles) const;

private:
    using Cells = std::vector<std::vector<int>>;

    Cells distribute(const Particles& particles) const;
    std::vector<std::vector<int>> buildAdjacency() const;
    void requireValidCellSide(const Particles& particles) const;
    int coordinateToIndex(double coordinate) const;
    int shift(int index, int offset) const;
    int cellCount() const;

    Domain domain_;
    int cellsPerSide_;
    double cellSide_;
    double interactionRadius_;
    std::vector<std::vector<int>> adjacency_;
};
