#include "neighbor_search.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace {

void addPair(NeighborList& neighbors, int first, int second) {
  neighbors[first].push_back(second);
  neighbors[second].push_back(first);
}

void sortEntries(NeighborList& neighbors) {
  for (std::vector<int>& entry : neighbors) {
    std::sort(entry.begin(), entry.end());
  }
}

// Reparte las particulas en las M*M celdas, por indice de celda plano.
std::vector<std::vector<int>> distribute(const Particles& particles,
                                         const Domain& domain,
                                         int cellsPerSide) {
  const double cellSide = domain.side / cellsPerSide;
  std::vector<std::vector<int>> cells(cellsPerSide * cellsPerSide);
  for (std::size_t id = 0; id < particles.size(); ++id) {
    const int row = cellIndex(particles[id].y, cellSide, cellsPerSide);
    const int column = cellIndex(particles[id].x, cellSide, cellsPerSide);
    cells[row * cellsPerSide + column].push_back(static_cast<int>(id));
  }
  return cells;
}

}  // namespace

std::size_t pairCount(const NeighborList& neighbors) {
  std::size_t total = 0;
  for (const std::vector<int>& entry : neighbors) {
    total += entry.size();
  }
  return total / 2;
}

int maxCellsPerSide(double side, double interactionRadius, double maxRadius) {
  const double reach = interactionRadius + 2.0 * maxRadius;
  return std::max(static_cast<int>(std::ceil(side / reach)) - 1, 1);
}

NeighborList bruteForceNeighbors(const Particles& particles,
                                 const Domain& domain,
                                 double interactionRadius) {
  NeighborList neighbors(particles.size());
  for (std::size_t first = 0; first < particles.size(); ++first) {
    for (std::size_t second = first + 1; second < particles.size(); ++second) {
      if (domain.areNeighbors(particles[first], particles[second],
                              interactionRadius)) {
        addPair(neighbors, static_cast<int>(first), static_cast<int>(second));
      }
    }
  }
  sortEntries(neighbors);
  return neighbors;
}

NeighborList cellIndexNeighbors(const Particles& particles,
                                const Domain& domain, int cellsPerSide,
                                double interactionRadius) {
  if (cellsPerSide < 1) {
    throw std::invalid_argument("M debe ser mayor o igual a 1");
  }
  const double maxRadius = largestRadius(particles);
  if (domain.side / cellsPerSide <= interactionRadius + 2.0 * maxRadius) {
    throw std::invalid_argument(
        "M=" + std::to_string(cellsPerSide) + " supera el maximo " +
        std::to_string(
            maxCellsPerSide(domain.side, interactionRadius, maxRadius)) +
        ": L/M debe ser mayor que rc + 2*rmax");
  }

  const std::vector<std::vector<int>> cells =
      distribute(particles, domain, cellsPerSide);
  NeighborList neighbors(particles.size());

  const auto pairUp = [&](int first, int second) {
    if (domain.areNeighbors(particles[first], particles[second],
                            interactionRadius)) {
      addPair(neighbors, first, second);
    }
  };

  for (int row = 0; row < cellsPerSide; ++row) {
    for (int column = 0; column < cellsPerSide; ++column) {
      const int cell = row * cellsPerSide + column;
      const std::vector<int>& here = cells[cell];

      for (std::size_t first = 0; first < here.size(); ++first) {
        for (std::size_t second = first + 1; second < here.size(); ++second) {
          pairUp(here[first], here[second]);
        }
      }

      // Solo contra celdas de indice mayor, para no repetir pares. Con M chico
      // y contorno periodico varios offsets caen en la misma celda, asi que
      // ademas se descartan las ya visitadas.
      int visited[8];
      int visitedCount = 0;
      for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
        for (int columnOffset = -1; columnOffset <= 1; ++columnOffset) {
          const int neighborRow =
              shiftedIndex(row, rowOffset, cellsPerSide, domain.periodic);
          const int neighborColumn =
              shiftedIndex(column, columnOffset, cellsPerSide, domain.periodic);
          if (neighborRow < 0 || neighborColumn < 0) {
            continue;
          }
          const int other = neighborRow * cellsPerSide + neighborColumn;
          if (other <= cell ||
              std::find(visited, visited + visitedCount, other) !=
                  visited + visitedCount) {
            continue;
          }
          visited[visitedCount++] = other;

          for (int first : here) {
            for (int second : cells[other]) {
              pairUp(first, second);
            }
          }
        }
      }
    }
  }

  sortEntries(neighbors);
  return neighbors;
}
