# TP1 — Búsqueda Eficiente de Partículas Vecinas (Cell Index Method)

Simulación de Sistemas — ITBA. Motor en C++, visualización en Python + matplotlib.

```
tp-1/
├── engine/           # C++: generación de partículas, CIM, fuerza bruta, timing
│   ├── include/      # particle, geometry, neighbor_search, io, arguments…
│   └── src/          # main + un archivo por comando (generate/neighbors/benchmark)
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

## Compilación

```bash
cd engine
make
```

El binario queda en `engine/build/cim`. Los comandos de abajo se ejecutan desde
`engine/`, por eso las rutas por defecto apuntan a `../data/`.

## Uso

Generar N partículas no superpuestas y escribir los archivos estático y dinámico:

```bash
./build/cim generate --n 1000 --l 20 --seed 1
```

Buscar vecinos y reportar el tiempo de ejecución:

```bash
./build/cim neighbors --method cim --m 13 --rc 1
./build/cim neighbors --method cim --periodic --particle 91
./build/cim neighbors --method brute
```

`--m` por defecto usa el máximo admitido. `--particle <id>` imprime los vecinos
de una partícula puntual, que es la que la visualización pinta de otro color.
`--verify` corre además fuerza bruta y compara ambas listas.

El tiempo reportado cubre la búsqueda completa, incluido el armado de las celdas:
ese costo crece con `M` y es parte de lo que hace que exista un `M` óptimo.

La salida (`data/neighbors.txt`) sigue el formato de la cátedra: una línea por
partícula, `id,vecino1,vecino2,...`, con ids desde 1.

Barridos de tiempo para las figuras:

```bash
./build/cim benchmark --sweep m --n 1000 --repeats 100
./build/cim benchmark --sweep n --n-min 10 --n-max 1000 --steps 10 --repeats 100
./build/cim benchmark --sweep n --n-min 10 --n-max 1000 --steps 10 --density 2.5
```

Cada corrida escribe un CSV con columnas `m,n,l,repeticiones,promedio_ms,desvio_ms`.
Con `--density` el lado se recalcula como `sqrt(N/densidad)` para mantener la
densidad constante; sin ella `L` queda fijo.

## Tamaño de celda

Con partículas puntuales alcanza con `L/M > rc`. Como acá los radios no son nulos,
el borde de una partícula puede caer en una celda vecina aunque su centro no, y el
alcance real entre centros pasa a ser `rc + r_i + r_j`. La condición se vuelve
entonces `L/M > rc + 2*rmax`, y el motor rechaza con error cualquier `M` que la viole.

## Estado

Motor completo: generación, CIM (paredes y periódico), fuerza bruta y benchmarks.
La salida fue verificada contra fuerza bruta para todos los `M` válidos en ambos
modos de contorno, y reproduce exactamente el archivo de referencia
`ArchivosEjemplo/AlgunosVecinos_100_rc6.txt`.

Pendiente: la visualización en Python.
