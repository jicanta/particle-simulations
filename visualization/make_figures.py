"""Corre el motor y genera todas las figuras del TP en un solo paso."""

import argparse
import subprocess
import sys
from pathlib import Path

import data_io
import plot_m
import plot_n
import plot_particles
import style

ROOT = Path(__file__).resolve().parent.parent
ENGINE = ROOT / "engine/build/cim"
DATA = ROOT / "data"

# N mas alto que entra en L=20 sin superponer, y un valor intermedio (punto 3).
N_MAXIMO = 1150
N_INTERMEDIO = 500
# N de la figura del punto 1: mas bajo para que se distingan las particulas.
N_FIGURA = 300
DENSIDAD_INTERMEDIA = 1.5
RC = 1.0
SEED = 1


def run(*arguments) -> None:
    command = [str(ENGINE), *[str(argument) for argument in arguments]]
    print("$", " ".join(command))
    subprocess.run(command, check=True, cwd=ROOT)


def build_data(repeats: int) -> None:
    run("generate", "--n", N_FIGURA, "--l", 20, "--seed", SEED,
        "--static", DATA / "static.txt", "--dynamic", DATA / "dynamic.txt")

    for boundary, output in (("paredes", "neighbors.txt"),
                             ("periodico", "neighbors_periodico.txt")):
        extra = ["--periodic"] if boundary == "periodico" else []
        run("neighbors", "--method", "cim", "--rc", RC,
            "--static", DATA / "static.txt", "--dynamic", DATA / "dynamic.txt",
            "--out", DATA / output, "--verify", *extra)

    for count, output in ((N_INTERMEDIO, "benchmark_m_intermedio.csv"),
                          (N_MAXIMO, "benchmark_m_maximo.csv")):
        run("benchmark", "--sweep", "m", "--n", count, "--l", 20, "--rc", RC,
            "--repeats", repeats, "--seed", SEED, "--out", DATA / output)

    # Densidad libre: L=20 fijo, asi que la densidad crece con N. El M optimo de
    # ese caso es el maximo admitido, que es tambien el que toma el motor por
    # defecto en el barrido a densidad fija (donde L cambia en cada paso).
    run("benchmark", "--sweep", "n", "--n-min", 10, "--n-max", N_MAXIMO,
        "--steps", 12, "--l", 20, "--rc", RC, "--repeats", repeats,
        "--seed", SEED, "--out", DATA / "benchmark_n_libre.csv")
    run("benchmark", "--sweep", "n", "--n-min", 10, "--n-max", N_MAXIMO,
        "--steps", 12, "--density", DENSIDAD_INTERMEDIA, "--rc", RC,
        "--repeats", repeats, "--seed", SEED,
        "--out", DATA / "benchmark_n_fija.csv")


def pick_particle(walls, periodic) -> int:
    """La particula que mas vecinos gana al pasar a contorno periodico.

    Asi las dos figuras muestran la misma particula y la diferencia entre los
    dos modos queda a la vista: los vecinos de mas estan del otro lado del area.
    """
    gained = [len(set(b) - set(a)) for a, b in zip(walls, periodic)]
    return max(range(len(gained)), key=lambda index: gained[index])


def build_figures(figures: Path, particle: int) -> None:
    system = data_io.read_system(DATA / "static.txt", DATA / "dynamic.txt")
    cells_per_side = data_io.max_cells_per_side(system.side, RC, system.max_radius)

    walls = data_io.read_neighbors(DATA / "neighbors.txt")
    periodic = data_io.read_neighbors(DATA / "neighbors_periodico.txt")
    reference = particle - 1 if particle else pick_particle(walls, periodic)
    print(f"particula de referencia: {reference + 1}")

    for neighbors, is_periodic, output in (
        (walls, False, "vecinos_paredes.png"),
        (periodic, True, "vecinos_periodico.png"),
    ):
        figure = plot_particles.plot(
            system, neighbors, reference, RC, cells_per_side, is_periodic,
            show_cells=True,
        )
        style.save(figure, figures / output)

    style.save(
        plot_m.plot([
            data_io.read_benchmark(DATA / "benchmark_m_intermedio.csv"),
            data_io.read_benchmark(DATA / "benchmark_m_maximo.csv"),
        ]),
        figures / "tiempo_vs_m.png",
    )

    style.save(
        plot_n.plot(
            data_io.read_benchmark(DATA / "benchmark_n_libre.csv"),
            data_io.read_benchmark(DATA / "benchmark_n_fija.csv"),
        ),
        figures / "tiempo_vs_n.png",
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repeats", type=int, default=100)
    parser.add_argument("--figures", type=Path, default=DATA / "figuras")
    parser.add_argument("--particle", type=int, default=0,
                        help="referencia de la figura del punto 1 "
                             "(0 elige una que gane vecinos por el contorno)")
    parser.add_argument("--skip-engine", action="store_true",
                        help="reusar los archivos que ya estan en data/")
    arguments = parser.parse_args()

    if not arguments.skip_engine:
        if not ENGINE.exists():
            sys.exit(f"falta el binario {ENGINE}: compilar con `make` en engine/")
        build_data(arguments.repeats)

    style.apply()
    build_figures(arguments.figures, arguments.particle)


if __name__ == "__main__":
    main()
