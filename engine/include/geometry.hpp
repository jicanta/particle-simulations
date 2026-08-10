#pragma once

#include "particle.hpp"

class Domain {
public:
    Domain(double side, bool periodic);

    double side() const;
    bool isPeriodic() const;

    double borderDistance(const Particle& first, const Particle& second) const;
    bool areNeighbors(const Particle& first, const Particle& second, double interactionRadius) const;

private:
    double axisDelta(double from, double to) const;
    double squaredCenterDistance(const Particle& first, const Particle& second) const;

    double side_;
    bool periodic_;
};
