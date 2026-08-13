"""Punto 1: posiciones de todas las particulas, con una de referencia y sus vecinos."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.lines import Line2D
from matplotlib.patches import Circle, Rectangle

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent


def _circles(system, ids, color, alpha=1.0, edge=None, width=0.8):
    patches = [
        Circle((system.x[i], system.y[i]), system.radius[i]) for i in ids
    ]
    return PatchCollection(
        patches,
        facecolors=color,
        edgecolors=edge if edge is not None else color,
        alpha=alpha,
        linewidths=width,
    )


def _draw_cells(axes, side, cells_per_side):
    cell_side = side / cells_per_side
    for index in range(1, cells_per_side):
        position = index * cell_side
        axes.plot([position, position], [0, side], color="#c8cdd4",
                  linewidth=0.5, zorder=0)
        axes.plot([0, side], [position, position], color="#c8cdd4",
                  linewidth=0.5, zorder=0)


def _draw_reach(axes, system, particle, interaction_radius, periodic):
    """Alcance de la particula de referencia: hasta donde llega su borde + rc.

    Con contorno periodico se dibujan ademas las 8 copias trasladadas, que son
    las que explican por que un vecino puede estar del otro lado del area.
    """
    reach = system.radius[particle] + interaction_radius
    shifts = [(0.0, 0.0)]
    if periodic:
        shifts += [
            (dx * system.side, dy * system.side)
            for dx in (-1, 0, 1)
            for dy in (-1, 0, 1)
            if (dx, dy) != (0, 0)
        ]

    for offset, (dx, dy) in enumerate(shifts):
        axes.add_patch(
            Circle(
                (system.x[particle] + dx, system.y[particle] + dy),
                reach,
                facecolor="none",
                edgecolor=style.REFERENCE,
                linestyle="--",
                linewidth=1.0,
                alpha=1.0 if offset == 0 else 0.5,
                zorder=4,
            )
        )


def plot(system, neighbors, particle, interaction_radius, cells_per_side,
         periodic, show_cells):
    figure, axes = plt.subplots(figsize=(7.0, 7.0))
    axes.grid(False)

    if show_cells:
        _draw_cells(axes, system.side, cells_per_side)

    axes.add_patch(
        Rectangle(
            (0, 0),
            system.side,
            system.side,
            facecolor="none",
            edgecolor="black",
            linewidth=1.2,
            zorder=5,
        )
    )

    related = set(neighbors[particle]) | {particle}
    others = [i for i in range(system.count) if i not in related]

    axes.add_collection(_circles(system, others, "white", edge=style.OTHER))
    axes.add_collection(
        _circles(system, neighbors[particle], style.NEIGHBOR, edge="#1a6b1a")
    )
    axes.add_collection(_circles(system, [particle], style.REFERENCE, edge="#8b1a1a"))
    _draw_reach(axes, system, particle, interaction_radius, periodic)

    margin = 0.05 * system.side
    axes.set_xlim(-margin, system.side + margin)
    axes.set_ylim(-margin, system.side + margin)
    axes.set_aspect("equal")
    axes.set_xlabel("x")
    axes.set_ylabel("y")

    boundary = "periodico" if periodic else "paredes"
    found = len(neighbors[particle])
    axes.set_title(
        f"N={system.count}, L={system.side:g}, rc={interaction_radius:g}, "
        f"M={cells_per_side}, contorno {boundary}\n"
        f"particula {particle + 1} y su{'s' if found != 1 else ''} {found} "
        f"vecino{'s' if found != 1 else ''}"
    )
    axes.legend(
        handles=[
            Line2D([], [], marker="o", linestyle="", markerfacecolor=style.REFERENCE,
                   markeredgecolor="#8b1a1a", label=f"particula {particle + 1}"),
            Line2D([], [], marker="o", linestyle="", markerfacecolor=style.NEIGHBOR,
                   markeredgecolor="#1a6b1a", label="vecinos"),
            Line2D([], [], marker="o", linestyle="", markerfacecolor="white",
                   markeredgecolor=style.OTHER, label="resto"),
            Line2D([], [], color=style.REFERENCE, linestyle="--",
                   label="alcance $r_i + r_c$\n(la distancia se mide\nborde a borde)"),
        ],
        loc="upper left",
        bbox_to_anchor=(1.02, 1.0),
    )
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--static", type=Path, default=ROOT / "data/static.txt")
    parser.add_argument("--dynamic", type=Path, default=ROOT / "data/dynamic.txt")
    parser.add_argument("--neighbors", type=Path, default=ROOT / "data/neighbors.txt")
    parser.add_argument("--particle", type=int, default=1,
                        help="id de la particula de referencia (desde 1)")
    parser.add_argument("--rc", type=float, default=1.0)
    parser.add_argument("--m", type=int, default=0, help="0 usa el maximo admitido")
    parser.add_argument("--periodic", action="store_true")
    parser.add_argument("--cells", action="store_true", help="dibujar la grilla MxM")
    parser.add_argument("--out", type=Path, default=ROOT / "data/figuras/vecinos.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    style.apply()
    system = data_io.read_system(arguments.static, arguments.dynamic)
    neighbors = data_io.read_neighbors(arguments.neighbors)
    if len(neighbors) != system.count:
        raise SystemExit(
            f"la lista de vecinos tiene {len(neighbors)} filas y el sistema "
            f"{system.count} particulas"
        )
    if not 1 <= arguments.particle <= system.count:
        raise SystemExit(f"la particula {arguments.particle} no existe")

    cells_per_side = arguments.m or data_io.max_cells_per_side(
        system.side, arguments.rc, system.max_radius
    )
    figure = plot(
        system,
        neighbors,
        arguments.particle - 1,
        arguments.rc,
        cells_per_side,
        arguments.periodic,
        arguments.cells,
    )
    style.save(figure, arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
