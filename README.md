# Tarea 2 — Torneo de Komikan Concurrente

---

## 1. Descripción del proyecto

Este programa simula un torneo de Komikan utilizando hilos y mecanismos de sincronización POSIX. El servidor recibe `N` jugadores, los empareja de dos en dos y administra un máximo de `M` mesas que pueden funcionar simultáneamente.

En cada partida, un jugador representa al **Puma** y el otro controla el bando de los **Perros**. El tablero tiene 28 posiciones: una cuadrícula principal de 5 × 5 y tres posiciones adicionales que forman el trono. Cada partida utiliza un Puma y doce Perros, representados por hilos independientes.

Las fichas intentan moverse después de retardos aleatorios, sin turnos fijos. Un Juez valida las solicitudes y coordina los cambios del tablero para evitar movimientos incompatibles.

El **Puma gana al capturar seis Perros**. Los **Perros ganan si el Puma queda sin movimientos ni capturas válidas**. El programa también contempla el abandono notificado de un jugador y el cierre del servidor con `Ctrl+C`.

---

## 2. Archivos del proyecto

```text
tarea2SO/
├── include/
│   ├── argumentos.hpp
│   ├── cfg.hpp
│   ├── cliente.hpp
│   ├── juez.hpp
│   ├── logs.hpp
│   ├── matchmaker.hpp
│   ├── mesa.hpp
│   ├── salaespera.hpp
│   └── tablero.hpp
├── src/
│   ├── argumentos.cpp
│   ├── cfg.cpp
│   ├── cliente.cpp
│   ├── fichas.cpp
│   ├── juez.cpp
│   ├── logs.cpp
│   ├── main.cpp
│   ├── matchmaker.cpp
│   ├── mesa.cpp
│   ├── salaespera.cpp
│   └── tablero.cpp
├── Makefile
├── config.txt
├── README.md
├── informe.pdf
└── .gitignore
```

El archivo `config.txt` está en la **carpeta raíz**. Es opcional al ejecutar el programa. El informe se entrega por separado en PDF, con un máximo de dos páginas, según el enunciado.

| Archivo o módulo | Para qué sirve |
|:---|:---|
| `argumentos` | Lee y valida los parámetros `N`, `M` y la ruta opcional del archivo de configuración. |
| `cfg` | Carga los retardos, la semilla y las posiciones iniciales de las fichas. |
| `cliente` | Representa a un jugador y permite notificar el término de su partida. |
| `salaespera` | Registra las llegadas y forma parejas Puma–Perros. |
| `matchmaker` | Controla el límite de mesas activas y administra sus identificadores. |
| `tablero` | Guarda las posiciones, conexiones y operaciones sobre las fichas. |
| `juez` | Valida movimientos, capturas y condiciones de victoria. |
| `mesa` | Coordina las solicitudes, el estado de la partida y su finalización. |
| `fichas.cpp` | Ejecuta los hilos de las trece fichas de cada partida. |
| `logs` | Muestra eventos y estados del tablero sin mezclar impresiones de distintas mesas. |
| `main.cpp` | Inicia el servidor, administra los hilos y coordina el cierre con `SIGINT`. |
| `Makefile` | Compila el proyecto y elimina los archivos generados durante la compilación. |

---

## 3. Compilación y ejecución

Se requiere **Linux u otro entorno Unix compatible con POSIX**, `g++` con soporte para **C++17** y **GNU Make**.

Desde la carpeta raíz del repositorio, compilar con:

```bash
make compilar
```

También se puede utilizar simplemente:

```bash
make
```

Ambos comandos generan el ejecutable `torneo` utilizando `-Wall -Wextra -std=c++17 -pthread`.

Para ejecutar el programa, después de compilar:

```bash
./torneo N M
```

Si se quiere utilizar el archivo de configuración de la raíz:

```bash
./torneo N M config.txt
```

Por ejemplo:

```bash
./torneo 10 2
./torneo 10 2 config.txt
```

| Parámetro | Significado |
|:---|:---|
| `N` | Cantidad total de jugadores. Debe ser un número entero par, mayor o igual a 2. |
| `M` | Máximo de mesas que pueden estar activas al mismo tiempo. Debe ser un entero positivo. |
| `config.txt` | Archivo opcional de configuración ubicado en la raíz del proyecto. |

Con `N=10`, se forman cinco partidas. Si `M=2`, se ejecutan como máximo dos partidas simultáneas; las demás parejas esperan a que se libere una mesa.

También es posible compilar y ejecutar con un solo comando:

```bash
make ejecutar N=10 M=2
make ejecutar N=10 M=2 CONFIG=config.txt
```

Para eliminar el ejecutable y los archivos intermedios de compilación:

```bash
make clean
```

Este comando borra `torneo` y la carpeta generada `build/`, pero **no elimina** los archivos `.cpp`, `.hpp`, el `README.md`, el informe ni `config.txt`.

---

## 4. Formato del archivo `config.txt`

El archivo se encuentra en la carpeta raíz del proyecto y utiliza una palabra clave seguida de sus valores, separados por espacios. Las posiciones del tablero se indican con números entre **0 y 27**.

Contenido de ejemplo:

```text
retardomin 50
retardomax 300
semilla 42
puma 25
perros 0 1 2 3 4 5 6 7 8 9 10 11
```

| Campo | Descripción |
|:---|:---|
| `retardomin` | Retardo mínimo en milisegundos. Debe ser mayor o igual a 0. |
| `retardomax` | Retardo máximo en milisegundos. Debe ser mayor o igual a `retardomin`. |
| `semilla` | Número utilizado para inicializar la generación de valores aleatorios. |
| `puma` | Posición inicial del Puma. |
| `perros` | Doce posiciones iniciales para los Perros, distintas entre sí y de la posición del Puma. |

Si se ejecuta sin `config.txt`, el programa utiliza sus valores predeterminados: retardos entre **50 y 300 ms**, semilla **1**, Puma en la posición **25** y doce Perros distribuidos de acuerdo con esa semilla.

Si se proporciona un archivo que no existe o contiene valores inválidos, el programa muestra un error y no inicia el torneo. La semilla ayuda a reproducir los valores aleatorios, aunque la concurrencia puede cambiar el orden de ejecución de los movimientos.

---

## 5. Funciones implementadas

### Servidor y emparejamiento

El servidor crea los hilos de los jugadores y los registra en `SalaEspera`. Los roles se asignan según el orden real de llegada. Cuando hay un Puma y un jugador Perros disponibles, se forma una pareja.

`Matchmaker` asigna una mesa libre a cada pareja, respetando el máximo `M`. Si todas están ocupadas, las siguientes parejas esperan. Al terminar una partida, su mesa se libera y puede ser utilizada nuevamente.

### Tablero, fichas y Juez

Cada partida tiene su propio `Tablero` y su propio Juez. Las trece fichas funcionan como hilos independientes y proponen movimientos después de retardos aleatorios.

El Juez comprueba las reglas antes de aceptar un movimiento: posición inicial correcta, destino libre, conexión válida y condiciones de captura del Puma. Después de los movimientos válidos, se revisan las condiciones de victoria.

### Registro y finalización

Los eventos de cada mesa se muestran en la terminal con su identificador. Se imprime el tablero inicial y el tablero actualizado después de cada movimiento válido. Las impresiones están protegidas para que los mensajes de varias mesas no se mezclen.

Si se notifica el abandono de un jugador, el Juez declara ganador al bando contrario y termina esa partida sin detener las demás. El programa contempla **abandonos notificados**; no implementa detección automática de cualquier fallo inesperado de un hilo.

Al presionar `Ctrl+C`, el servidor recibe `SIGINT`, solicita el cierre de las partidas activas y espera a que finalicen los hilos antes de liberar los recursos.

---

## 6. Decisiones de diseño

### Uso de hilos POSIX

Se utiliza `pthread_create()` para ejecutar jugadores, mesas, jueces y fichas de forma concurrente. Cada partida mantiene su propio estado, por lo que las mesas pueden funcionar de manera independiente.

### Límite de mesas y espera pasiva

`Matchmaker` utiliza un semáforo POSIX para respetar el máximo de mesas simultáneas. Cuando no hay cupos, el hilo queda bloqueado con `sem_wait()` hasta que se libere una mesa.

`SalaEspera` utiliza una variable de condición con `pthread_cond_wait()` cuando todavía no existe una pareja completa.

### Consistencia del tablero

Las fichas consultan una copia del tablero y envían sus solicitudes a una cola compartida de la partida. El Juez vuelve a validar cada solicitud sobre el estado actualizado y los cambios se realizan protegidos por el mutex de `Mesa`.

Así se evita que dos solicitudes aceptadas ocupen simultáneamente la misma posición y se mantiene la consistencia de cada tablero.

### Cierre y liberación de recursos

Cuando una partida termina, se detienen sus solicitudes, se espera la finalización de los hilos y se notifica a los jugadores. Luego se libera la mesa para que pueda comenzar otra partida.

Para `Ctrl+C`, se utiliza un hilo que espera la señal con `sigwait()`. El servidor deja de asignar partidas, notifica el cierre y espera a los hilos antes de destruir los recursos compartidos.

---

## 7. Integrantes

- Javier Zavaleta
- Martina Veloso