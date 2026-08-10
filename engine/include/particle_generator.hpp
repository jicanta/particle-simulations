#pragma once

#include "geometry.hpp"
#include "particle.hpp"

struct GenerationRequest {
    int count = 0;
    double minRadius = 0.23;
    double maxRadius = 0.26;
    double property = 1.0;
    unsigned int seed = 1;
};

Particles generateParticles(const GenerationRequest& request, const Domain& domain);
