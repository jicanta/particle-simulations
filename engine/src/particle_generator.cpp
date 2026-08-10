#include "particle_generator.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int kAttemptsPerParticle = 2000;

class PlacementGrid {
public:
    PlacementGrid(const Domain& domain, const Particles& placed, double cellSide)
        : domain_(domain),
          placed_(placed),
          cellsPerSide_(std::max(1, static_cast<int>(domain.side() / cellSide))),
          cellSide_(domain.side() / cellsPerSide_),
          cells_(static_cast<std::size_t>(cellsPerSide_) * cellsPerSide_) {}

    bool overlaps(const Particle& candidate) const {
        const int row = coordinateToIndex(candidate.y);
        const int column = coordinateToIndex(candidate.x);
        for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
            for (int columnOffset = -1; columnOffset <= 1; ++columnOffset) {
                const int neighborRow = shift(row, rowOffset);
                const int neighborColumn = shift(column, columnOffset);
                if (neighborRow < 0 || neighborColumn < 0) {
                    continue;
                }
                for (int placedId : cells_[cellIndex(neighborRow, neighborColumn)]) {
                    if (domain_.borderDistance(candidate, placed_[static_cast<std::size_t>(placedId)]) <= 0.0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void insert(int particleId, const Particle& particle) {
        const int row = coordinateToIndex(particle.y);
        const int column = coordinateToIndex(particle.x);
        cells_[cellIndex(row, column)].push_back(particleId);
    }

private:
    int coordinateToIndex(double coordinate) const {
        const int index = static_cast<int>(coordinate / cellSide_);
        return std::min(std::max(index, 0), cellsPerSide_ - 1);
    }

    int shift(int index, int offset) const {
        const int shifted = index + offset;
        if (shifted >= 0 && shifted < cellsPerSide_) {
            return shifted;
        }
        if (!domain_.isPeriodic()) {
            return -1;
        }
        return (shifted + cellsPerSide_) % cellsPerSide_;
    }

    std::size_t cellIndex(int row, int column) const {
        return static_cast<std::size_t>(row) * cellsPerSide_ + column;
    }

    const Domain& domain_;
    const Particles& placed_;
    int cellsPerSide_;
    double cellSide_;
    std::vector<std::vector<int>> cells_;
};

}  // namespace

Particles generateParticles(const GenerationRequest& request, const Domain& domain) {
    if (request.count <= 0) {
        throw std::invalid_argument("N debe ser mayor que cero");
    }
    if (2.0 * request.maxRadius >= domain.side()) {
        throw std::invalid_argument("el radio de las particulas no entra en el area");
    }

    std::mt19937 generator(request.seed);
    std::uniform_real_distribution<double> radiusRange(request.minRadius, request.maxRadius);

    Particles particles;
    particles.reserve(static_cast<std::size_t>(request.count));
    PlacementGrid grid(domain, particles, 2.0 * request.maxRadius);

    const long long attemptLimit = static_cast<long long>(request.count) * kAttemptsPerParticle;
    for (long long attempt = 0; static_cast<int>(particles.size()) < request.count; ++attempt) {
        if (attempt >= attemptLimit) {
            throw std::runtime_error("no se pudieron ubicar " + std::to_string(request.count) +
                                     " particulas sin superponer en un area de lado " +
                                     std::to_string(domain.side()));
        }

        Particle candidate;
        candidate.radius = radiusRange(generator);
        candidate.property = request.property;

        std::uniform_real_distribution<double> positionRange(candidate.radius, domain.side() - candidate.radius);
        candidate.x = positionRange(generator);
        candidate.y = positionRange(generator);

        if (grid.overlaps(candidate)) {
            continue;
        }

        particles.push_back(candidate);
        grid.insert(static_cast<int>(particles.size()) - 1, candidate);
    }

    return particles;
}
