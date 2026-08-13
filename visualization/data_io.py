from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass
class System:
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


@dataclass
class Benchmark:
    m: np.ndarray
    n: np.ndarray
    side: np.ndarray
    repeats: np.ndarray
    mean_ms: np.ndarray
    deviation_ms: np.ndarray

    @property
    def density(self) -> np.ndarray:
        return self.n / self.side**2


def _rows_with_content(path: Path) -> list[list[str]]:
    with open(path) as stream:
        return [line.replace(",", " ").split() for line in stream if line.split()]


def _require_at_least(rows: list, expected: int, path: Path) -> None:
    if len(rows) < expected:
        raise ValueError(
            f"{path}: se esperaban {expected} lineas con contenido y hay {len(rows)}"
        )


def read_system(static_path: Path, dynamic_path: Path) -> System:
    static_rows = _rows_with_content(static_path)
    dynamic_rows = _rows_with_content(dynamic_path)

    _require_at_least(static_rows, 2, static_path)
    count = int(float(static_rows[0][0]))
    side = float(static_rows[1][0])
    _require_at_least(static_rows, 2 + count, static_path)
    _require_at_least(dynamic_rows, 1 + count, dynamic_path)

    radius = np.array([float(row[0]) for row in static_rows[2 : 2 + count]])
    positions = np.array(
        [[float(row[0]), float(row[1])] for row in dynamic_rows[1 : 1 + count]]
    )

    return System(
        side=side,
        time=float(dynamic_rows[0][0]),
        x=positions[:, 0],
        y=positions[:, 1],
        radius=radius,
    )


def read_neighbors(path: Path) -> list[list[int]]:
    return [
        [int(field) - 1 for field in row[1:]] for row in _rows_with_content(path)
    ]


def read_benchmark(path: Path) -> Benchmark:
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
    reach_between_centers = interaction_radius + 2.0 * max_radius
    return max(int(np.ceil(side / reach_between_centers)) - 1, 1)
