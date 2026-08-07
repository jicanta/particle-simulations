# TP1 — Búsqueda Eficiente de Partículas Vecinas (Cell Index Method)

Simulación de Sistemas — ITBA. Motor en C++, visualización en Python + matplotlib.

```
tp-1/
├── engine/           # C++: generación de partículas, CIM, fuerza bruta, timing
├── visualization/    # Python: figuras y curvas de tiempo
├── data/             # entrada y salida generadas (ignorado por git)
└── ArchivosEjemplo/  # formatos de referencia de la cátedra
```

El límite entre ambos lenguajes son archivos de texto: C++ escribe en `data/`,
Python solo lee de ahí.

## Parámetros por defecto

`L=20`, `rc=1`, `r_i = U[0.23, 0.26]`, partículas no superpuestas.
Distancia borde a borde: `dist(centros) - r_i - r_j < rc`.
Dos modos de contorno: paredes y condiciones periódicas.

## Estado

Estructura creada. Sin código todavía.
