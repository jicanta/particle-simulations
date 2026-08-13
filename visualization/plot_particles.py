import argparse
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.lines import Line2D
from matplotlib.patches import Circle, Rectangle

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent

CELL_GRID = {"color": "#c8cdd4", "linewidth": 0.5, "zorder": 0}
REACH_RING = {
    "facecolor": "none",
    "edgecolor": style.REFERENCE,
    "linestyle": "--",
    "linewidth": 1.0,
    "zorder": 4,
}
REFERENCE_EDGE = "#8b1a1a"
NEIGHBOR_EDGE = "#1a6b1a"


def _particle_circles(system, ids, fill, edge):
    return PatchCollection(
        [Circle((system.x[i], system.y[i]), system.radius[i]) for i in ids],
        facecolors=fill,
        edgecolors=edge,
        linewidths=0.8,
    )


def _periodic_image_offsets(side):
    return [
        (horizontal * side, vertical * side)
        for horizontal in (-1, 0, 1)
        for vertical in (-1, 0, 1)
        if (horizontal, vertical) != (0, 0)
    ]


def _draw_cell_grid(axes, side, cells_per_side):
    for index in range(1, cells_per_side):
        position = index * side / cells_per_side
        axes.plot([position, position], [0, side], **CELL_GRID)
        axes.plot([0, side], [position, position], **CELL_GRID)


def _draw_reach(axes, system, particle, interaction_radius, periodic):
    x, y = system.x[particle], system.y[particle]
    reach = system.radius[particle] + interaction_radius

    if periodic:
        for shift_x, shift_y in _periodic_image_offsets(system.side):
            axes.add_patch(
                Circle((x + shift_x, y + shift_y), reach, alpha=0.5, **REACH_RING)
            )
    axes.add_patch(Circle((x, y), reach, **REACH_RING))


def _legend_handles(particle):
    marker = {"marker": "o", "linestyle": ""}
    return [
        Line2D([], [], markerfacecolor=style.REFERENCE, markeredgecolor=REFERENCE_EDGE,
               label=f"particula {particle + 1}", **marker),
        Line2D([], [], markerfacecolor=style.NEIGHBOR, markeredgecolor=NEIGHBOR_EDGE,
               label="vecinos", **marker),
        Line2D([], [], markerfacecolor="white", markeredgecolor=style.OTHER,
               label="resto", **marker),
        Line2D([], [], color=style.REFERENCE, linestyle="--",
               label="alcance $r_i + r_c$\n(la distancia se mide\nborde a borde)"),
    ]


def plot(system, neighbors, particle, interaction_radius, cells_per_side, *,
         periodic, draw_cells):
    figure, axes = plt.subplots(figsize=(7.0, 7.0))
    axes.grid(False)

    if draw_cells:
        _draw_cell_grid(axes, system.side, cells_per_side)

    axes.add_patch(
        Rectangle((0, 0), system.side, system.side, facecolor="none",
                  edgecolor="black", linewidth=1.2, zorder=5)
    )

    highlighted = set(neighbors[particle]) | {particle}
    rest = [i for i in range(system.count) if i not in highlighted]

    axes.add_collection(_particle_circles(system, rest, "white", style.OTHER))
    axes.add_collection(
        _particle_circles(system, neighbors[particle], style.NEIGHBOR, NEIGHBOR_EDGE)
    )
    axes.add_collection(
        _particle_circles(system, [particle], style.REFERENCE, REFERENCE_EDGE)
    )
    _draw_reach(axes, system, particle, interaction_radius, periodic)

    margin = 0.05 * system.side
    axes.set_xlim(-margin, system.side + margin)
    axes.set_ylim(-margin, system.side + margin)
    axes.set_aspect("equal")
    axes.set_xlabel("x")
    axes.set_ylabel("y")

    found = len(neighbors[particle])
    axes.set_title(
        f"N={system.count}, L={system.side:g}, rc={interaction_radius:g}, "
        f"M={cells_per_side}, contorno {'periodico' if periodic else 'paredes'}\n"
        f"particula {particle + 1} y "
        f"{'1 vecino' if found == 1 else f'{found} vecinos'}"
    )
    axes.legend(handles=_legend_handles(particle), loc="upper left",
                bbox_to_anchor=(1.02, 1.0))
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Punto 1: posiciones de todas las particulas, con una de referencia y sus vecinos."
    )
    parser.add_argument("--static", type=Path, default=ROOT / "data/static.txt")
    parser.add_argument("--dynamic", type=Path, default=ROOT / "data/dynamic.txt")
    parser.add_argument("--neighbors", type=Path, default=ROOT / "data/neighbors.txt")
    parser.add_argument("--particle", type=int, default=1,
                        help="id de la particula de referencia, desde 1")
    parser.add_argument("--rc", type=float, default=1.0)
    parser.add_argument("--m", type=int,
                        help="celdas por lado; por omision el maximo admitido")
    parser.add_argument("--periodic", action="store_true")
    parser.add_argument("--cells", action="store_true", help="dibujar la grilla MxM")
    parser.add_argument("--out", type=Path, default=ROOT / "data/figuras/vecinos.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    system = data_io.read_system(arguments.static, arguments.dynamic)
    neighbors = data_io.read_neighbors(arguments.neighbors)
    if len(neighbors) != system.count:
        raise SystemExit(
            f"{arguments.neighbors} tiene {len(neighbors)} filas y el sistema "
            f"{system.count} particulas"
        )
    if not 1 <= arguments.particle <= system.count:
        raise SystemExit(
            f"la particula {arguments.particle} no existe: hay {system.count}"
        )
    if arguments.m is not None and arguments.m < 1:
        raise SystemExit("M debe ser mayor o igual a 1")

    cells_per_side = (
        arguments.m
        if arguments.m is not None
        else data_io.max_cells_per_side(system.side, arguments.rc, system.max_radius)
    )

    style.apply()
    figure = plot(
        system,
        neighbors,
        arguments.particle - 1,
        arguments.rc,
        cells_per_side,
        periodic=arguments.periodic,
        draw_cells=arguments.cells,
    )
    style.save(figure, arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
