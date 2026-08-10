#include "geometry.hpp"

#include <cmath>

Domain::Domain(double side, bool periodic) : side_(side), periodic_(periodic) {}

double Domain::side() const {
    return side_;
}

bool Domain::isPeriodic() const {
    return periodic_;
}

double Domain::axisDelta(double from, double to) const {
    double delta = to - from;
    if (!periodic_) {
        return delta;
    }
    if (delta > side_ / 2.0) {
        delta -= side_;
    } else if (delta < -side_ / 2.0) {
        delta += side_;
    }
    return delta;
}

double Domain::squaredCenterDistance(const Particle& first, const Particle& second) const {
    const double deltaX = axisDelta(first.x, second.x);
    const double deltaY = axisDelta(first.y, second.y);
    return deltaX * deltaX + deltaY * deltaY;
}

double Domain::borderDistance(const Particle& first, const Particle& second) const {
    return std::sqrt(squaredCenterDistance(first, second)) - first.radius - second.radius;
}

bool Domain::areNeighbors(const Particle& first, const Particle& second, double interactionRadius) const {
    const double reach = interactionRadius + first.radius + second.radius;
    return squaredCenterDistance(first, second) < reach * reach;
}
