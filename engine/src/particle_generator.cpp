#include "particle_generator.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int kAttemptsPerParticle = 2000;

}  // namespace

Particles generateParticles(const GenerationRequest& request,
                            const Domain& domain) {
  if (request.count <= 0) {
    throw std::invalid_argument("N debe ser mayor que cero");
  }
  if (2.0 * request.maxRadius >= domain.side) {
    throw std::invalid_argument(
        "el radio de las particulas no entra en el area");
  }

  // Grilla auxiliar de celdas de lado >= 2*rmax: dos particulas solo pueden
  // solaparse si estan en celdas contiguas.
  const int cellsPerSide =
      std::max(1, static_cast<int>(domain.side / (2.0 * request.maxRadius)));
  const double cellSide = domain.side / cellsPerSide;
  std::vector<std::vector<int>> cells(cellsPerSide * cellsPerSide);

  Particles particles;
  particles.reserve(static_cast<std::size_t>(request.count));

  const auto overlaps = [&](const Particle& candidate) {
    const int row = cellIndex(candidate.y, cellSide, cellsPerSide);
    const int column = cellIndex(candidate.x, cellSide, cellsPerSide);
    for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
      for (int columnOffset = -1; columnOffset <= 1; ++columnOffset) {
        const int neighborRow =
            shiftedIndex(row, rowOffset, cellsPerSide, domain.periodic);
        const int neighborColumn =
            shiftedIndex(column, columnOffset, cellsPerSide, domain.periodic);
        if (neighborRow < 0 || neighborColumn < 0) {
          continue;
        }
        for (int placed : cells[neighborRow * cellsPerSide + neighborColumn]) {
          if (domain.borderDistance(candidate, particles[placed]) <= 0.0) {
            return true;
          }
        }
      }
    }
    return false;
  };

  std::mt19937 generator(request.seed);
  std::uniform_real_distribution<double> radiusRange(request.minRadius,
                                                     request.maxRadius);

  const long long attemptLimit =
      static_cast<long long>(request.count) * kAttemptsPerParticle;
  for (long long attempt = 0;
       static_cast<int>(particles.size()) < request.count; ++attempt) {
    if (attempt >= attemptLimit) {
      throw std::runtime_error(
          "no se pudieron ubicar " + std::to_string(request.count) +
          " particulas sin superponer en un area de lado " +
          std::to_string(domain.side));
    }

    Particle candidate;
    candidate.radius = radiusRange(generator);
    candidate.property = request.property;

    std::uniform_real_distribution<double> positionRange(
        candidate.radius, domain.side - candidate.radius);
    candidate.x = positionRange(generator);
    candidate.y = positionRange(generator);

    if (overlaps(candidate)) {
      continue;
    }

    const int row = cellIndex(candidate.y, cellSide, cellsPerSide);
    const int column = cellIndex(candidate.x, cellSide, cellsPerSide);
    cells[row * cellsPerSide + column].push_back(
        static_cast<int>(particles.size()));
    particles.push_back(candidate);
  }

  return particles;
}
