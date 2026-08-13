# TP1 — Búsqueda Eficiente de Partículas Vecinas (Cell Index Method)

Simulación de Sistemas — ITBA. Motor en C++, visualización en Python + matplotlib.

```
tp-1/
├── engine/           # C++: generación de partículas, CIM, fuerza bruta, timing
│   ├── include/      # particle, geometry, neighbor_search, io, arguments…
│   └── src/          # main + un archivo por comando (generate/neighbors/benchmark)
├── visualization/    # Python: figuras y curvas de tiempo (matplotlib)
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

## Visualización

Python 3 con `numpy` y `matplotlib`. Los scripts leen únicamente los archivos de
`data/` y escriben las figuras en `data/figuras/`.

Todo de una vez (corre el motor y genera las cuatro figuras del TP):

```bash
cd visualization
python3 make_figures.py
```

Con `--skip-engine` reusa los archivos que ya están en `data/`, y `--repeats`
cambia la cantidad de mediciones por punto (100 por defecto).

Cada figura también se puede generar por separado:

```bash
# Punto 1: sistema completo, partícula de referencia y sus vecinos
python3 plot_particles.py --particle 91 --rc 1 --cells
python3 plot_particles.py --particle 91 --neighbors ../data/neighbors_periodico.txt --periodic

# Punto 3: tiempo vs M, una curva por archivo
python3 plot_m.py ../data/benchmark_m_intermedio.csv ../data/benchmark_m_maximo.csv

# Punto 4: tiempo vs N, densidad libre y densidad fija superpuestas
python3 plot_n.py --libre ../data/benchmark_n_libre.csv --fija ../data/benchmark_n_fija.csv
```

En la figura de partículas el círculo punteado marca el alcance `r_i + rc` de la
partícula de referencia: como la distancia se mide borde a borde, un vecino
cuenta si su borde toca ese círculo, no su centro. Con `--periodic` se dibujan
además las ocho copias trasladadas del alcance, que son las que explican los
vecinos que quedan del otro lado del área. `--cells` superpone la grilla M×M.

`make_figures.py` elige como referencia la partícula que más vecinos gana al
pasar a contorno periódico, así las dos figuras muestran el mismo caso y la
diferencia entre ambos modos queda a la vista; `--particle <id>` fuerza otra.

Los archivos de `data/` no guardan `rc`, `M` ni el tipo de contorno, así que
esos valores hay que pasárselos al script iguales a como se corrió el motor: en
`plot_particles.py` cambian la figura, y en `plot_m.py` / `plot_n.py` el `--rc`
solo aparece en el título.

## Tamaño de celda

Con partículas puntuales alcanza con `L/M > rc`. Como acá los radios no son nulos,
el borde de una partícula puede caer en una celda vecina aunque su centro no, y el
alcance real entre centros pasa a ser `rc + r_i + r_j`. La condición se vuelve
entonces `L/M > rc + 2*rmax`, y el motor rechaza con error cualquier `M` que la viole.

## Resultados

Con `L=20`, `rc=1` y `r_i = U[0.23, 0.26]` el máximo `M` admitido es 13 y el
máximo `N` que entra sin superponer es ~1150.

- **Punto 3** (`tiempo_vs_m.png`): el tiempo cae fuerte de `M=1` (fuerza bruta) a
  `M≈8` y después se aplana. El óptimo queda cerca del máximo admitido (11-13,
  la diferencia entre esos valores está dentro de las barras de error): más
  celdas achican las listas a comparar, pero armarlas cuesta cada vez más.
- **Punto 4** (`tiempo_vs_n.png`): a densidad fija el tiempo crece casi lineal
  con `N` (exponente ajustado ≈1.2) y a densidad libre (`L=20` fijo) crece
  bastante más rápido (≈1.6), porque al aumentar la densidad cada celda contiene
  más partículas.

## Estado

Motor y visualización completos: generación, CIM (paredes y periódico), fuerza
bruta, benchmarks y las figuras de los puntos 1, 3 y 4. La salida fue verificada
contra fuerza bruta para todos los `M` válidos en ambos modos de contorno, y
reproduce exactamente el archivo de referencia
`ArchivosEjemplo/AlgunosVecinos_100_rc6.txt`.
