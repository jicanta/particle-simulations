#include "cell_index_method.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

int CellIndexMethod::maxCellsPerSide(double side, double interactionRadius, double maxRadius) {
    const double reach = interactionRadius + 2.0 * maxRadius;
    const int cells = static_cast<int>(std::ceil(side / reach)) - 1;
    return std::max(cells, 1);
}

CellIndexMethod::CellIndexMethod(const Domain& domain, int cellsPerSide, double interactionRadius)
    : domain_(domain),
      cellsPerSide_(cellsPerSide),
      cellSide_(domain.side() / cellsPerSide),
      interactionRadius_(interactionRadius) {
    if (cellsPerSide_ < 1) {
        throw std::invalid_argument("M debe ser mayor o igual a 1");
    }
    adjacency_ = buildAdjacency();
}

int CellIndexMethod::cellCount() const {
    return cellsPerSide_ * cellsPerSide_;
}

int CellIndexMethod::coordinateToIndex(double coordinate) const {
    const int index = static_cast<int>(coordinate / cellSide_);
    return std::min(std::max(index, 0), cellsPerSide_ - 1);
}

int CellIndexMethod::shift(int index, int offset) const {
    const int shifted = index + offset;
    if (shifted >= 0 && shifted < cellsPerSide_) {
        return shifted;
    }
    if (!domain_.isPeriodic()) {
        return -1;
    }
    return (shifted + cellsPerSide_) % cellsPerSide_;
}

std::vector<std::vector<int>> CellIndexMethod::buildAdjacency() const {
    std::vector<std::vector<int>> adjacency(static_cast<std::size_t>(cellCount()));
    for (int row = 0; row < cellsPerSide_; ++row) {
        for (int column = 0; column < cellsPerSide_; ++column) {
            const int cell = row * cellsPerSide_ + column;
            std::vector<int>& higher = adjacency[static_cast<std::size_t>(cell)];
            for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
                for (int columnOffset = -1; columnOffset <= 1; ++columnOffset) {
                    const int neighborRow = shift(row, rowOffset);
                    const int neighborColumn = shift(column, columnOffset);
                    if (neighborRow < 0 || neighborColumn < 0) {
                        continue;
                    }
                    const int neighbor = neighborRow * cellsPerSide_ + neighborColumn;
                    if (neighbor > cell) {
                        higher.push_back(neighbor);
                    }
                }
            }
            std::sort(higher.begin(), higher.end());
            higher.erase(std::unique(higher.begin(), higher.end()), higher.end());
        }
    }
    return adjacency;
}

void CellIndexMethod::requireValidCellSide(const Particles& particles) const {
    const double reach = interactionRadius_ + 2.0 * largestRadius(particles);
    if (cellSide_ <= reach) {
        throw std::invalid_argument(
            "M=" + std::to_string(cellsPerSide_) + " supera el maximo " +
            std::to_string(maxCellsPerSide(domain_.side(), interactionRadius_, largestRadius(particles))) +
            ": L/M debe ser mayor que rc + 2*rmax");
    }
}

CellIndexMethod::Cells CellIndexMethod::distribute(const Particles& particles) const {
    Cells cells(static_cast<std::size_t>(cellCount()));
    for (std::size_t particleId = 0; particleId < particles.size(); ++particleId) {
        const int row = coordinateToIndex(particles[particleId].y);
        const int column = coordinateToIndex(particles[particleId].x);
        cells[static_cast<std::size_t>(row * cellsPerSide_ + column)].push_back(static_cast<int>(particleId));
    }
    return cells;
}

NeighborList CellIndexMethod::findNeighbors(const Particles& particles) const {
    requireValidCellSide(particles);

    NeighborList neighbors(particles.size());
    const Cells cells = distribute(particles);

    for (int cell = 0; cell < cellCount(); ++cell) {
        const std::vector<int>& here = cells[static_cast<std::size_t>(cell)];

        for (std::size_t first = 0; first < here.size(); ++first) {
            for (std::size_t second = first + 1; second < here.size(); ++second) {
                if (domain_.areNeighbors(particles[static_cast<std::size_t>(here[first])],
                                         particles[static_cast<std::size_t>(here[second])],
                                         interactionRadius_)) {
                    neighbors.addPair(here[first], here[second]);
                }
            }
        }

        for (int other : adjacency_[static_cast<std::size_t>(cell)]) {
            for (int first : here) {
                for (int second : cells[static_cast<std::size_t>(other)]) {
                    if (domain_.areNeighbors(particles[static_cast<std::size_t>(first)],
                                             particles[static_cast<std::size_t>(second)],
                                             interactionRadius_)) {
                        neighbors.addPair(first, second);
                    }
                }
            }
        }
    }

    neighbors.sortEachEntry();
    return neighbors;
}
