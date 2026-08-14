#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "commands.hpp"
#include "neighbor_search.hpp"
#include "particle_generator.hpp"
#include "stopwatch.hpp"

namespace {

struct Measurement {
  double mean = 0.0;
  double deviation = 0.0;
};

Measurement summarize(const std::vector<double>& samples) {
  Measurement measurement;
  for (double sample : samples) {
    measurement.mean += sample;
  }
  measurement.mean /= static_cast<double>(samples.size());

  if (samples.size() < 2) {
    return measurement;
  }

  double squaredError = 0.0;
  for (double sample : samples) {
    squaredError += (sample - measurement.mean) * (sample - measurement.mean);
  }
  measurement.deviation =
      std::sqrt(squaredError / static_cast<double>(samples.size() - 1));
  return measurement;
}

Measurement measureSearch(const Particles& particles, const Domain& domain,
                          double interactionRadius, int cellsPerSide,
                          const std::string& method, int repeats) {
  std::vector<double> samples;
  samples.reserve(static_cast<std::size_t>(repeats));
  for (int repeat = 0; repeat < repeats; ++repeat) {
    const Stopwatch stopwatch;
    const NeighborList neighbors =
        method == "brute"
            ? bruteForceNeighbors(particles, domain, interactionRadius)
            : cellIndexNeighbors(particles, domain, cellsPerSide,
                                 interactionRadius);
    samples.push_back(stopwatch.elapsedMilliseconds());
  }
  return summarize(samples);
}

class ResultWriter {
 public:
  explicit ResultWriter(const std::string& path) : file_(path) {
    if (!file_) {
      throw std::runtime_error("no se pudo abrir para escritura: " + path);
    }
    writeAndEcho("m,n,l,repeticiones,promedio_ms,desvio_ms");
  }

  void add(int cellsPerSide, int count, double side, int repeats,
           const Measurement& measurement) {
    std::ostringstream line;
    line << std::setprecision(10) << cellsPerSide << "," << count << "," << side
         << "," << repeats << "," << measurement.mean << ","
         << measurement.deviation;
    writeAndEcho(line.str());
  }

 private:
  void writeAndEcho(const std::string& line) {
    file_ << line << "\n";
    std::cout << line << "\n";
  }

  std::ofstream file_;
};

void sweepCells(const Arguments& arguments, GenerationRequest request,
                const std::string& method, double interactionRadius,
                int repeats, bool periodic) {
  const Domain domain{arguments.number("l", 20.0), periodic};
  request.count = arguments.requiredInteger("n");
  const Particles particles = generateParticles(request, domain);
  const int maxCells =
      maxCellsPerSide(domain.side, interactionRadius, largestRadius(particles));

  ResultWriter writer(arguments.text("out", "../data/benchmark_m.csv"));
  for (int cellsPerSide = 1; cellsPerSide <= maxCells; ++cellsPerSide) {
    writer.add(cellsPerSide, request.count, domain.side, repeats,
               measureSearch(particles, domain, interactionRadius, cellsPerSide,
                             method, repeats));
  }
}

void sweepCount(const Arguments& arguments, GenerationRequest request,
                const std::string& method, double interactionRadius,
                int repeats, bool periodic) {
  const int minimum = arguments.integer("n-min", 10);
  const int maximum = arguments.requiredInteger("n-max");
  const int steps = arguments.integer("steps", 10);
  const double density = arguments.number("density", 0.0);
  const double fixedSide = arguments.number("l", 20.0);
  if (steps < 1) {
    throw std::invalid_argument("--steps debe ser mayor o igual a 1");
  }

  ResultWriter writer(arguments.text("out", "../data/benchmark_n.csv"));
  for (int step = 0; step < steps; ++step) {
    request.count = steps == 1
                        ? maximum
                        : minimum + (maximum - minimum) * step / (steps - 1);
    const double side =
        density > 0.0 ? std::sqrt(request.count / density) : fixedSide;
    const Domain domain{side, periodic};

    const Particles particles = generateParticles(request, domain);
    const int maxCells =
        maxCellsPerSide(side, interactionRadius, largestRadius(particles));
    const int cellsPerSide =
        std::min(arguments.integer("m", maxCells), maxCells);

    writer.add(cellsPerSide, request.count, side, repeats,
               measureSearch(particles, domain, interactionRadius, cellsPerSide,
                             method, repeats));
  }
}

}  // namespace

int runBenchmark(const Arguments& arguments) {
  const std::string sweep = arguments.text("sweep", "m");
  const std::string method = arguments.text("method", "cim");
  if (method != "cim" && method != "brute") {
    throw std::invalid_argument("metodo desconocido: " + method);
  }

  const double interactionRadius = arguments.number("rc", 1.0);
  const int repeats = arguments.integer("repeats", 100);
  if (repeats < 1) {
    throw std::invalid_argument("--repeats debe ser mayor o igual a 1");
  }
  const bool periodic = arguments.has("periodic");

  GenerationRequest request;
  request.minRadius = arguments.number("min-radius", 0.23);
  request.maxRadius = arguments.number("max-radius", 0.26);
  request.seed = static_cast<unsigned int>(arguments.integer("seed", 1));

  if (sweep == "m") {
    sweepCells(arguments, request, method, interactionRadius, repeats,
               periodic);
  } else if (sweep == "n") {
    sweepCount(arguments, request, method, interactionRadius, repeats,
               periodic);
  } else {
    throw std::invalid_argument("barrido desconocido: " + sweep);
  }
  return 0;
}
