#include <iostream>
#include <stdexcept>

#include "commands.hpp"
#include "io.hpp"
#include "neighbor_search.hpp"
#include "stopwatch.hpp"

int runNeighbors(const Arguments& arguments) {
  const SystemState state =
      readSystem(arguments.text("static", "../data/static.txt"),
                 arguments.text("dynamic", "../data/dynamic.txt"));

  const Domain domain{state.side, arguments.has("periodic")};
  const double interactionRadius = arguments.number("rc", 1.0);
  const std::string method = arguments.text("method", "cim");
  if (method != "cim" && method != "brute") {
    throw std::invalid_argument("metodo desconocido: " + method);
  }

  const int maxCells = maxCellsPerSide(state.side, interactionRadius,
                                       largestRadius(state.particles));
  const int cellsPerSide = arguments.integer("m", maxCells);

  const Stopwatch stopwatch;
  const NeighborList neighbors =
      method == "brute"
          ? bruteForceNeighbors(state.particles, domain, interactionRadius)
          : cellIndexNeighbors(state.particles, domain, cellsPerSide,
                               interactionRadius);
  const double elapsed = stopwatch.elapsedMilliseconds();

  const std::string outputPath = arguments.text("out", "../data/neighbors.txt");
  writeNeighbors(outputPath, neighbors);

  std::cout << "metodo: " << method << "\n"
            << "N: " << state.particles.size() << "\n"
            << "L: " << state.side << "\n"
            << "rc: " << interactionRadius << "\n"
            << "contorno: " << (domain.periodic ? "periodico" : "paredes")
            << "\n";
  if (method == "cim") {
    std::cout << "M: " << cellsPerSide << " (maximo " << maxCells << ")\n";
  }
  std::cout << "pares vecinos: " << pairCount(neighbors) << "\n"
            << "tiempo: " << elapsed << " ms\n"
            << "salida: " << outputPath << "\n";

  if (arguments.has("particle")) {
    const int particleId = arguments.requiredInteger("particle");
    if (particleId < 1 || particleId > static_cast<int>(neighbors.size())) {
      throw std::invalid_argument("la particula " + std::to_string(particleId) +
                                  " no existe");
    }
    std::cout << "vecinos de " << particleId << ":";
    for (int neighbor : neighbors[particleId - 1]) {
      std::cout << " " << neighbor + 1;
    }
    std::cout << "\n";
  }

  if (arguments.has("verify")) {
    const bool matches =
        neighbors ==
        bruteForceNeighbors(state.particles, domain, interactionRadius);
    std::cout << "verificacion contra fuerza bruta: "
              << (matches ? "coincide" : "difiere") << "\n";
    return matches ? 0 : 1;
  }
  return 0;
}
