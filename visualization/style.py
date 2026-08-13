from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter

OTHER = "#9aa7b8"
REFERENCE = "#d62728"
NEIGHBOR = "#2ca02c"
CURVES = ("#1f77b4", "#d62728", "#2ca02c", "#ff7f0e")

DECADE_SUBDIVISIONS = (1.0, 2.0, 5.0)
DENSE_DECADE_SUBDIVISIONS = (1.0, 2.0, 3.0, 5.0, 7.0)


def apply() -> None:
    plt.rcParams.update(
        {
            "figure.figsize": (7.0, 5.0),
            "figure.dpi": 120,
            "font.size": 11,
            "axes.titlesize": 12,
            "axes.labelsize": 12,
            "axes.grid": True,
            "grid.alpha": 0.3,
            "grid.linestyle": ":",
            "legend.frameon": True,
            "legend.framealpha": 0.9,
            "lines.markersize": 5,
        }
    )


def plain_log_ticks(axis, subdivisions=DECADE_SUBDIVISIONS) -> None:
    axis.set_major_locator(LogLocator(base=10.0, subs=subdivisions))
    axis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:g}"))
    axis.set_minor_formatter(NullFormatter())


def save(figure, path: Path) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(path, bbox_inches="tight")
    print(f"figura: {path}")
