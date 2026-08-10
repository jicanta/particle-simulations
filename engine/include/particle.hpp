#pragma once

#include <vector>

struct Particle {
    double x = 0.0;
    double y = 0.0;
    double radius = 0.0;
    double property = 0.0;
};

using Particles = std::vector<Particle>;

double largestRadius(const Particles& particles);
