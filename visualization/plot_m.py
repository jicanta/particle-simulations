import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent


def plot(benchmarks, interaction_radius):
    figure, axes = plt.subplots()

    for index, benchmark in enumerate(benchmarks):
        color = style.CURVES[index % len(style.CURVES)]
        axes.errorbar(
            benchmark.m,
            benchmark.mean_ms,
            yerr=benchmark.deviation_ms,
            marker="o",
            capsize=3,
            color=color,
            label=f"N = {int(benchmark.n[0])}",
        )

        fastest = int(benchmark.mean_ms.argmin())
        axes.plot(benchmark.m[fastest], benchmark.mean_ms[fastest], marker="*",
                  markersize=14, linestyle="", color=color, zorder=5)
        axes.annotate(
            f"M optimo = {int(benchmark.m[fastest])}",
            (benchmark.m[fastest], benchmark.mean_ms[fastest]),
            textcoords="offset points",
            xytext=(-10, -18),
            ha="right",
            color=color,
        )

    first = benchmarks[0]
    axes.set_yscale("log")
    style.plain_log_ticks(axes.yaxis, style.DENSE_DECADE_SUBDIVISIONS)
    axes.set_xticks(np.unique(np.concatenate([b.m for b in benchmarks])))
    axes.set_xlabel("M (celdas por lado)")
    axes.set_ylabel("tiempo de busqueda [ms]")
    axes.set_title(
        f"Tiempo en funcion de M (L={first.side[0]:g}, rc={interaction_radius:g}, "
        f"{int(first.repeats[0])} repeticiones)\nM=1 equivale a fuerza bruta"
    )
    axes.legend()
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Punto 3: tiempo de computo en funcion de M, con barra de error."
    )
    parser.add_argument("inputs", nargs="+", type=Path,
                        help="CSV de barrido de M, uno por valor de N")
    parser.add_argument("--rc", type=float, default=1.0,
                        help="solo para el titulo: el CSV no guarda rc")
    parser.add_argument("--out", type=Path,
                        default=ROOT / "data/figuras/tiempo_vs_m.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    style.apply()
    benchmarks = [data_io.read_benchmark(path) for path in arguments.inputs]
    style.save(plot(benchmarks, arguments.rc), arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
