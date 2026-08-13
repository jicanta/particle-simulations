"""Punto 4: tiempo de computo en funcion de N, a densidad libre y a densidad fija."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

import data_io
import style

ROOT = Path(__file__).resolve().parent.parent

# Con N chico el tiempo lo domina el armado de las celdas y no la busqueda, asi
# que el exponente se ajusta recien a partir de este N.
FIT_FROM = 100


def _exponent(benchmark) -> float:
    """Pendiente de log(t) vs log(N): el exponente con el que crece el metodo."""
    usable = (benchmark.n >= FIT_FROM) & (benchmark.mean_ms > 0)
    if usable.sum() < 2:
        return float("nan")
    slope, _ = np.polyfit(
        np.log(benchmark.n[usable]), np.log(benchmark.mean_ms[usable]), 1
    )
    return float(slope)


def plot(free, fixed):
    figure, axes = plt.subplots()

    curves = [
        (free, style.CURVES[0], "o", f"densidad libre (L = {free.side[0]:g})"),
        (fixed, style.CURVES[1], "s",
         f"densidad fija (N/L² = {fixed.density[-1]:.2f})"),
    ]

    for benchmark, color, marker, label in curves:
        axes.errorbar(
            benchmark.n,
            benchmark.mean_ms,
            yerr=benchmark.deviation_ms,
            marker=marker,
            capsize=3,
            color=color,
            label=f"{label}\n     t $\\propto$ N$^{{{_exponent(benchmark):.2f}}}$",
        )

    repeats = int(free.repeats[0])
    axes.set_xscale("log")
    axes.set_yscale("log")
    style.log_ticks(axes.xaxis)
    style.log_ticks(axes.yaxis)
    axes.set_xlabel("N (cantidad de particulas)")
    axes.set_ylabel("tiempo de busqueda [ms]")
    axes.set_title(
        f"Tiempo en funcion de N con el M optimo (rc=1, {repeats} repeticiones)\n"
        f"exponentes ajustados sobre N ≥ {FIT_FROM}"
    )
    axes.legend(loc="upper left")
    return figure


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--libre", type=Path,
                        default=ROOT / "data/benchmark_n_libre.csv",
                        help="barrido con L fijo (la densidad crece con N)")
    parser.add_argument("--fija", type=Path,
                        default=ROOT / "data/benchmark_n_fija.csv",
                        help="barrido con L creciente (densidad constante)")
    parser.add_argument("--out", type=Path, default=ROOT / "data/figuras/tiempo_vs_n.png")
    parser.add_argument("--show", action="store_true")
    arguments = parser.parse_args()

    style.apply()
    figure = plot(
        data_io.read_benchmark(arguments.libre),
        data_io.read_benchmark(arguments.fija),
    )
    style.save(figure, arguments.out)
    if arguments.show:
        plt.show()


if __name__ == "__main__":
    main()
