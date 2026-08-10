#include "brute_force.hpp"

NeighborList bruteForceNeighbors(const Particles& particles, const Domain& domain, double interactionRadius) {
    NeighborList neighbors(particles.size());
    for (std::size_t first = 0; first < particles.size(); ++first) {
        for (std::size_t second = first + 1; second < particles.size(); ++second) {
            if (domain.areNeighbors(particles[first], particles[second], interactionRadius)) {
                neighbors.addPair(static_cast<int>(first), static_cast<int>(second));
            }
        }
    }
    neighbors.sortEachEntry();
    return neighbors;
}
