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

SIDE = 20.0
INTERACTION_RADIUS = 1.0
SEED = 1

MOST_PARTICLES_THAT_FIT = 1150
MIDDLE_PARTICLE_COUNT = 500
PARTICLES_IN_SYSTEM_FIGURE = 300
INTERMEDIATE_DENSITY = 1.5
SMALLEST_PARTICLE_COUNT = 10
SWEEP_STEPS = 12


def run_engine(*arguments) -> None:
    command = [str(ENGINE), *[str(argument) for argument in arguments]]
    print("$", " ".join(command))
    subprocess.run(command, check=True, cwd=ROOT)


def build_data(repeats: int) -> None:
    run_engine("generate", "--n", PARTICLES_IN_SYSTEM_FIGURE, "--l", SIDE,
               "--seed", SEED, "--static", DATA / "static.txt",
               "--dynamic", DATA / "dynamic.txt")

    for periodic, output in ((False, "neighbors.txt"), (True, "neighbors_periodico.txt")):
        run_engine("neighbors", "--method", "cim", "--rc", INTERACTION_RADIUS,
                   "--static", DATA / "static.txt", "--dynamic", DATA / "dynamic.txt",
                   "--out", DATA / output, "--verify",
                   *(["--periodic"] if periodic else []))

    for count, output in ((MIDDLE_PARTICLE_COUNT, "benchmark_m_intermedio.csv"),
                          (MOST_PARTICLES_THAT_FIT, "benchmark_m_maximo.csv")):
        run_engine("benchmark", "--sweep", "m", "--n", count, "--l", SIDE,
                   "--rc", INTERACTION_RADIUS, "--repeats", repeats, "--seed", SEED,
                   "--out", DATA / output)

    sweep_range = ("--n-min", SMALLEST_PARTICLE_COUNT,
                   "--n-max", MOST_PARTICLES_THAT_FIT, "--steps", SWEEP_STEPS)
    run_engine("benchmark", "--sweep", "n", *sweep_range, "--l", SIDE,
               "--rc", INTERACTION_RADIUS, "--repeats", repeats, "--seed", SEED,
               "--out", DATA / "benchmark_n_libre.csv")
    run_engine("benchmark", "--sweep", "n", *sweep_range,
               "--density", INTERMEDIATE_DENSITY, "--rc", INTERACTION_RADIUS,
               "--repeats", repeats, "--seed", SEED,
               "--out", DATA / "benchmark_n_fija.csv")


def neighbors_gained_with_periodic(walls, periodic) -> list[int]:
    return [
        len(set(with_periodic) - set(with_walls))
        for with_walls, with_periodic in zip(walls, periodic)
    ]


def build_figures(figures: Path, chosen_particle: int | None) -> None:
    system = data_io.read_system(DATA / "static.txt", DATA / "dynamic.txt")
    cells_per_side = data_io.max_cells_per_side(
        system.side, INTERACTION_RADIUS, system.max_radius
    )

    walls = data_io.read_neighbors(DATA / "neighbors.txt")
    periodic = data_io.read_neighbors(DATA / "neighbors_periodico.txt")
    gained = neighbors_gained_with_periodic(walls, periodic)
    if chosen_particle is not None and not 1 <= chosen_particle <= system.count:
        raise SystemExit(
            f"la particula {chosen_particle} no existe: hay {system.count}"
        )
    reference = (
        chosen_particle - 1
        if chosen_particle is not None
        else max(range(len(gained)), key=gained.__getitem__)
    )
    print(
        f"particula de referencia: {reference + 1} "
        f"(+{gained[reference]} vecinos con contorno periodico)"
    )

    for neighbors, is_periodic, output in (
        (walls, False, "vecinos_paredes.png"),
        (periodic, True, "vecinos_periodico.png"),
    ):
        style.save(
            plot_particles.plot(system, neighbors, reference, INTERACTION_RADIUS,
                                cells_per_side, periodic=is_periodic, draw_cells=True),
            figures / output,
        )

    style.save(
        plot_m.plot(
            [
                data_io.read_benchmark(DATA / "benchmark_m_intermedio.csv"),
                data_io.read_benchmark(DATA / "benchmark_m_maximo.csv"),
            ],
            INTERACTION_RADIUS,
        ),
        figures / "tiempo_vs_m.png",
    )
    style.save(
        plot_n.plot(
            data_io.read_benchmark(DATA / "benchmark_n_libre.csv"),
            data_io.read_benchmark(DATA / "benchmark_n_fija.csv"),
            INTERACTION_RADIUS,
        ),
        figures / "tiempo_vs_n.png",
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Corre el motor y genera todas las figuras del TP en un solo paso."
    )
    parser.add_argument("--repeats", type=int, default=100)
    parser.add_argument("--figures", type=Path, default=DATA / "figuras")
    parser.add_argument("--particle", type=int,
                        help="referencia de la figura del punto 1; por omision "
                             "la que mas vecinos gana con contorno periodico")
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
