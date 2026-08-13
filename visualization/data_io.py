"""Lectura de los archivos de texto que escribe el motor en C++."""

from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass
class System:
    """Estado del sistema en un instante: geometria y particulas."""

    side: float
    time: float
    x: np.ndarray
    y: np.ndarray
    radius: np.ndarray

    @property
    def count(self) -> int:
        return int(self.x.size)

    @property
    def max_radius(self) -> float:
        return float(self.radius.max())

    @property
    def density(self) -> float:
        return self.count / self.side**2


@dataclass
class Benchmark:
    """Un barrido de tiempos: una fila por valor del parametro estudiado."""

    m: np.ndarray
    n: np.ndarray
    side: np.ndarray
    repeats: np.ndarray
    mean_ms: np.ndarray
    deviation_ms: np.ndarray

    @property
    def density(self) -> np.ndarray:
        return self.n / self.side**2


def _rows(path: Path):
    """Lineas con contenido, ya separadas en campos."""
    with open(path) as stream:
        for line in stream:
            fields = line.replace(",", " ").split()
            if fields:
                yield fields


def read_system(static_path: Path, dynamic_path: Path) -> System:
    """Combina el archivo estatico (N, L, radios) con el dinamico (posiciones)."""
    static_rows = _rows(static_path)
    count = int(float(next(static_rows)[0]))
    side = float(next(static_rows)[0])
    radius = np.array([float(next(static_rows)[0]) for _ in range(count)])

    dynamic_rows = _rows(dynamic_path)
    time = float(next(dynamic_rows)[0])
    positions = np.array(
        [[float(value) for value in next(dynamic_rows)[:2]] for _ in range(count)]
    )

    return System(side, time, positions[:, 0], positions[:, 1], radius)


def read_neighbors(path: Path) -> list[list[int]]:
    """Vecinos de cada particula, con los ids pasados a base 0."""
    entries = []
    for fields in _rows(path):
        entries.append([int(field) - 1 for field in fields[1:]])
    return entries


def read_benchmark(path: Path) -> Benchmark:
    """CSV con columnas m,n,l,repeticiones,promedio_ms,desvio_ms."""
    table = np.atleast_1d(np.genfromtxt(path, delimiter=",", names=True))
    return Benchmark(
        m=table["m"],
        n=table["n"],
        side=table["l"],
        repeats=table["repeticiones"],
        mean_ms=table["promedio_ms"],
        deviation_ms=table["desvio_ms"],
    )


def max_cells_per_side(side: float, interaction_radius: float, max_radius: float) -> int:
    """Mismo criterio que el motor: L/M > rc + 2*rmax."""
    reach = interaction_radius + 2.0 * max_radius
    return max(int(np.ceil(side / reach)) - 1, 1)
