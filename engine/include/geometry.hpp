#pragma once

#include "particle.hpp"

// Area cuadrada de lado L, con paredes o con condiciones periodicas.
struct Domain {
  double side = 0.0;
  bool periodic = false;

  // Distancia borde a borde: dist(centros) - r_i - r_j.
  double borderDistance(const Particle& first, const Particle& second) const;
  bool areNeighbors(const Particle& first, const Particle& second,
                    double interactionRadius) const;
};

// Fila (o columna) de la grilla que contiene a una coordenada.
int cellIndex(double coordinate, double cellSide, int cellsPerSide);

// Fila (o columna) corrida en offset: envuelve si el contorno es periodico y
// devuelve -1 si queda fuera del area.
int shiftedIndex(int index, int offset, int cellsPerSide, bool periodic);
