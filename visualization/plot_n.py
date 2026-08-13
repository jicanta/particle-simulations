import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent

SMALLEST_N_IN_FIT = 100


def _fitted_exponent(benchmark) -> float:
    inside_fit = (benchmark.n >= SMALLEST_N_IN_FIT) & (benchmark.mean_ms > 0)
    if inside_fit.sum() < 2:
        return float("nan")
    slope, _ = np.polyfit(
        np.log(benchmark.n[inside_fit]), np.log(benchmark.mean_ms[inside_fit]), 1
    )
    return float(slope)


def plot(free_density, fixed_density, interaction_radius):
    figure, axes = plt.subplots()

    curves = (
        (free_density, style.CURVES[0], "o",
         f"densidad libre (L = {free_density.side[0]:g})"),
        (fixed_density, style.CURVES[1], "s",
         f"densidad fija (N/L² = {fixed_density.density.mean():.2f})"),
    )

    for benchmark, color, marker, label in curves:
        axes.errorbar(
            benchmark.n,
            benchmark.mean_ms,
            yerr=benchmark.deviation_ms,
            marker=marker,
            capsize=3,
            color=color,
            label=f"{label}\nt $\\propto$ N$^{{{_fitted_exponent(benchmark):.2f}}}$",
        )

    axes.set_xscale("log")
    axes.set_yscale("log")
    style.plain_log_ticks(axes.xaxis)
    style.plain_log_ticks(axes.yaxis)
    axes.set_xlabel("N (cantidad de particulas)")
    axes.set_ylabel("tiempo de busqueda [ms]")
    axes.set_title(
        f"Tiempo en funcion de N con el M optimo (rc={interaction_radius:g}, "
        f"{int(free_density.repeats[0])} repeticiones)\n"
        f"exponentes ajustados sobre N ≥ {SMALLEST_N_IN_FIT}"
    )
    axes.legend(loc="upper left")
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Punto 4: tiempo de computo en funcion de N, a densidad libre y a densidad fija."
    )
    parser.add_argument("--libre", type=Path,
                        default=ROOT / "data/benchmark_n_libre.csv",
                        help="barrido con L fijo, la densidad crece con N")
    parser.add_argument("--fija", type=Path,
                        default=ROOT / "data/benchmark_n_fija.csv",
                        help="barrido con L creciente, densidad constante")
    parser.add_argument("--rc", type=float, default=1.0,
                        help="solo para el titulo: el CSV no guarda rc")
    parser.add_argument("--out", type=Path,
                        default=ROOT / "data/figuras/tiempo_vs_n.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    style.apply()
    figure = plot(
        data_io.read_benchmark(arguments.libre),
        data_io.read_benchmark(arguments.fija),
        arguments.rc,
    )
    style.save(figure, arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
