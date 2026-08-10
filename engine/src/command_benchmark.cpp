#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "brute_force.hpp"
#include "cell_index_method.hpp"
#include "commands.hpp"
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
    measurement.deviation = std::sqrt(squaredError / static_cast<double>(samples.size() - 1));
    return measurement;
}

Measurement measureSearch(const Particles& particles, const Domain& domain, double interactionRadius,
                          int cellsPerSide, const std::string& method, int repeats) {
    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(repeats));

    if (method == "brute") {
        for (int repeat = 0; repeat < repeats; ++repeat) {
            Stopwatch stopwatch;
            const NeighborList neighbors = bruteForceNeighbors(particles, domain, interactionRadius);
            samples.push_back(stopwatch.elapsedMilliseconds());
        }
    } else {
        const CellIndexMethod cellIndexMethod(domain, cellsPerSide, interactionRadius);
        for (int repeat = 0; repeat < repeats; ++repeat) {
            Stopwatch stopwatch;
            const NeighborList neighbors = cellIndexMethod.findNeighbors(particles);
            samples.push_back(stopwatch.elapsedMilliseconds());
        }
    }

    return summarize(samples);
}

class ResultWriter {
public:
    explicit ResultWriter(const std::string& path) : file_(path) {
        if (!file_) {
            throw std::runtime_error("no se pudo abrir para escritura: " + path);
        }
        emit("m,n,l,repeticiones,promedio_ms,desvio_ms");
    }

    void add(int cellsPerSide, int count, double side, int repeats, const Measurement& measurement) {
        std::ostringstream line;
        line << std::setprecision(10) << cellsPerSide << "," << count << "," << side << "," << repeats << ","
             << measurement.mean << "," << measurement.deviation;
        emit(line.str());
    }

private:
    void emit(const std::string& line) {
        file_ << line << "\n";
        std::cout << line << "\n";
    }

    std::ofstream file_;
};

}  // namespace

int runBenchmark(const Arguments& arguments) {
    const std::string sweep = arguments.text("sweep", "m");
    const std::string method = arguments.text("method", "cim");
    const double interactionRadius = arguments.number("rc", 1.0);
    const int repeats = arguments.integer("repeats", 100);
    const bool periodic = arguments.has("periodic");
    const unsigned int seed = arguments.unsignedInteger("seed", 1);

    GenerationRequest request;
    request.minRadius = arguments.number("min-radius", 0.23);
    request.maxRadius = arguments.number("max-radius", 0.26);
    request.seed = seed;

    if (sweep == "m") {
        const int count = arguments.requiredInteger("n");
        const Domain domain(arguments.number("l", 20.0), periodic);
        request.count = count;
        const Particles particles = generateParticles(request, domain);
        const int maxCells = CellIndexMethod::maxCellsPerSide(domain.side(), interactionRadius, largestRadius(particles));

        ResultWriter writer(arguments.text("out", "../data/benchmark_m.csv"));
        for (int cellsPerSide = 1; cellsPerSide <= maxCells; ++cellsPerSide) {
            writer.add(cellsPerSide, count, domain.side(), repeats,
                       measureSearch(particles, domain, interactionRadius, cellsPerSide, method, repeats));
        }
        return 0;
    }

    if (sweep == "n") {
        const int minimum = arguments.integer("n-min", 10);
        const int maximum = arguments.requiredInteger("n-max");
        const int steps = arguments.integer("steps", 10);
        const double density = arguments.number("density", 0.0);
        const double fixedSide = arguments.number("l", 20.0);

        ResultWriter writer(arguments.text("out", "../data/benchmark_n.csv"));
        for (int step = 0; step < steps; ++step) {
            const int count = steps == 1 ? maximum
                                         : minimum + (maximum - minimum) * step / (steps - 1);
            const double side = density > 0.0 ? std::sqrt(static_cast<double>(count) / density) : fixedSide;
            const Domain domain(side, periodic);

            request.count = count;
            const Particles particles = generateParticles(request, domain);
            const int maxCells = CellIndexMethod::maxCellsPerSide(side, interactionRadius, largestRadius(particles));
            const int cellsPerSide = std::min(arguments.integer("m", maxCells), maxCells);

            writer.add(cellsPerSide, count, side, repeats,
                       measureSearch(particles, domain, interactionRadius, cellsPerSide, method, repeats));
        }
        return 0;
    }

    throw std::invalid_argument("barrido desconocido: " + sweep);
}
