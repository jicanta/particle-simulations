#include <iostream>

#include "commands.hpp"
#include "io.hpp"
#include "particle_generator.hpp"

int runGenerate(const Arguments& arguments) {
    const Domain domain(arguments.number("l", 20.0), arguments.has("periodic"));

    GenerationRequest request;
    request.count = arguments.requiredInteger("n");
    request.minRadius = arguments.number("min-radius", 0.23);
    request.maxRadius = arguments.number("max-radius", 0.26);
    request.seed = arguments.unsignedInteger("seed", 1);

    SystemState state;
    state.particles = generateParticles(request, domain);
    state.side = domain.side();
    state.time = 0.0;

    const std::string staticPath = arguments.text("static", "../data/static.txt");
    const std::string dynamicPath = arguments.text("dynamic", "../data/dynamic.txt");
    writeSystem(staticPath, dynamicPath, state);

    std::cout << "particulas: " << state.particles.size() << "\n"
              << "L: " << state.side << "\n"
              << "estatico: " << staticPath << "\n"
              << "dinamico: " << dynamicPath << "\n";
    return 0;
}
