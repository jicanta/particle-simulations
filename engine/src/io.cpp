#include "io.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

std::ifstream openForReading(const std::string& path) {
  std::ifstream stream(path);
  if (!stream) {
    throw std::runtime_error("no se pudo abrir para lectura: " + path);
  }
  return stream;
}

std::ofstream openForWriting(const std::string& path) {
  std::ofstream stream(path);
  if (!stream) {
    throw std::runtime_error("no se pudo abrir para escritura: " + path);
  }
  stream << std::setprecision(10);
  return stream;
}

std::istringstream nextLineWithContent(std::istream& stream,
                                       const std::string& path) {
  std::string line;
  while (std::getline(stream, line)) {
    if (line.find_first_not_of(" \t\r") != std::string::npos) {
      return std::istringstream(line);
    }
  }
  throw std::runtime_error("archivo incompleto: " + path);
}

}

SystemState readSystem(const std::string& staticPath,
                       const std::string& dynamicPath) {
  std::ifstream staticStream = openForReading(staticPath);

  int count = 0;
  nextLineWithContent(staticStream, staticPath) >> count;
  if (count <= 0) {
    throw std::runtime_error("cantidad de particulas invalida en: " +
                             staticPath);
  }

  SystemState state;
  nextLineWithContent(staticStream, staticPath) >> state.side;
  state.particles.resize(static_cast<std::size_t>(count));

  for (Particle& particle : state.particles) {
    std::istringstream fields = nextLineWithContent(staticStream, staticPath);
    fields >> particle.radius;
    if (!(fields >> particle.property)) {
      particle.property = 1.0;
    }
  }

  std::ifstream dynamicStream = openForReading(dynamicPath);
  nextLineWithContent(dynamicStream, dynamicPath) >> state.time;
  for (Particle& particle : state.particles) {
    nextLineWithContent(dynamicStream, dynamicPath) >> particle.x >> particle.y;
  }

  return state;
}

void writeSystem(const std::string& staticPath, const std::string& dynamicPath,
                 const SystemState& state) {
  std::ofstream staticStream = openForWriting(staticPath);
  staticStream << state.particles.size() << "\n" << state.side << "\n";
  for (const Particle& particle : state.particles) {
    staticStream << particle.radius << " " << particle.property << "\n";
  }

  std::ofstream dynamicStream = openForWriting(dynamicPath);
  dynamicStream << state.time << "\n";
  for (const Particle& particle : state.particles) {
    dynamicStream << particle.x << " " << particle.y << "\n";
  }
}

void writeNeighbors(const std::string& path, const NeighborList& neighbors) {
  std::ofstream stream = openForWriting(path);
  for (std::size_t id = 0; id < neighbors.size(); ++id) {
    stream << id + 1;
    for (int neighbor : neighbors[id]) {
      stream << "," << neighbor + 1;
    }
    stream << "\n";
  }
}
