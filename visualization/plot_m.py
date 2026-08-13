"""Punto 3: tiempo de computo en funcion de M, con barra de error."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent


def plot(benchmarks):
    figure, axes = plt.subplots()

    for index, benchmark in enumerate(benchmarks):
        color = style.CURVES[index % len(style.CURVES)]
        count = int(benchmark.n[0])
        axes.errorbar(
            benchmark.m,
            benchmark.mean_ms,
            yerr=benchmark.deviation_ms,
            marker="o",
            capsize=3,
            color=color,
            label=f"N = {count}",
        )

        best = int(benchmark.mean_ms.argmin())
        axes.plot(
            benchmark.m[best],
            benchmark.mean_ms[best],
            marker="*",
            markersize=14,
            color=color,
            linestyle="",
            zorder=5,
        )
        axes.annotate(
            f"M optimo = {int(benchmark.m[best])}",
            (benchmark.m[best], benchmark.mean_ms[best]),
            textcoords="offset points",
            xytext=(-10, -18),
            ha="right",
            color=color,
        )

    reference = benchmarks[0]
    repeats = int(reference.repeats[0])
    side = reference.side[0]
    axes.set_yscale("log")
    style.log_ticks(axes.yaxis, subs=(1.0, 2.0, 3.0, 5.0, 7.0))
    axes.set_xticks(reference.m)
    axes.set_xlabel("M (celdas por lado)")
    axes.set_ylabel("tiempo de busqueda [ms]")
    axes.set_title(
        f"Tiempo en funcion de M (L={side:g}, rc=1, {repeats} repeticiones)\n"
        "M=1 equivale a fuerza bruta"
    )
    axes.legend()
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path,
                        help="CSV de barrido de M, uno por valor de N")
    parser.add_argument("--out", type=Path, default=ROOT / "data/figuras/tiempo_vs_m.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    style.apply()
    figure = plot([data_io.read_benchmark(path) for path in arguments.inputs])
    style.save(figure, arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
