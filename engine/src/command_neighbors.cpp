#include <iostream>
#include <stdexcept>

#include "brute_force.hpp"
#include "commands.hpp"
#include "io.hpp"
#include "stopwatch.hpp"

int runNeighbors(const Arguments& arguments) {
    const SystemState state = readSystem(arguments.text("static", "../data/static.txt"),
                                         arguments.text("dynamic", "../data/dynamic.txt"));

    const Domain domain(state.side, arguments.has("periodic"));
    const double interactionRadius = arguments.number("rc", 1.0);
    const std::string method = arguments.text("method", "brute");

    if (method != "brute") {
        throw std::invalid_argument("metodo desconocido: " + method);
    }

    Stopwatch stopwatch;
    const NeighborList neighbors = bruteForceNeighbors(state.particles, domain, interactionRadius);
    const double elapsed = stopwatch.elapsedMilliseconds();

    const std::string outputPath = arguments.text("out", "../data/neighbors.txt");
    writeNeighbors(outputPath, neighbors);

    std::cout << "metodo: " << method << "\n"
              << "N: " << state.particles.size() << "\n"
              << "L: " << state.side << "\n"
              << "rc: " << interactionRadius << "\n"
              << "contorno: " << (domain.isPeriodic() ? "periodico" : "paredes") << "\n"
              << "pares vecinos: " << neighbors.pairCount() << "\n"
              << "tiempo: " << elapsed << " ms\n"
              << "salida: " << outputPath << "\n";
    return 0;
}
