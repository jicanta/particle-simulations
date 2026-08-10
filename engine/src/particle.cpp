#include "particle.hpp"

#include <algorithm>

double largestRadius(const Particles& particles) {
    double largest = 0.0;
    for (const Particle& particle : particles) {
        largest = std::max(largest, particle.radius);
    }
    return largest;
}
