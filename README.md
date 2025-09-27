# 🎮 Proyecto GALAGA (Versión Consola en C++)

Este proyecto es una implementación en **C++** de un menú interactivo inspirado en el clásico juego **Galaga**, utilizando la terminal para mostrar gráficos ASCII y colores ANSI.

## ✨ Características

- 🎨 **Pantallas decorativas** con marcos y colores.
- 🚀 **Pantalla de inicio (Splash Screen)** con arte ASCII del logo de GALAGA.
- 🎮 **Controles representados con iconos ASCII** (mover izquierda, derecha, disparar).
- ⭐ **Objetivo del juego** explicado en pantalla.
- 📋 **Menú principal**:
  - Iniciar partida (simulador)
  - Ver puntajes
  - Salir
- 🕹️ **Simulador de juego**:
  - S: Sumar 100 puntos (simula eliminar enemigo)
  - M: Morir y terminar partida
  - Q: Salir sin guardar puntaje
- 🏆 **Sistema de puntajes**:
  - Registro automático de nombre al terminar
  - Tabla ordenada de mayor a menor puntaje
  - Sin persistencia (se reinicia al cerrar programa)

## 📂 Estructura del código

- **Galaga.cpp**: Archivo principal con toda la lógica del juego
- **Pantalla.h/cpp**: Clase para manejo de pantalla (requiere ncurses)
- **Nave.h/cpp**: Clase para la nave del jugador
- *Funciones de consola*: manejo de cursor, colores y entrada de teclado
- *Sistema de puntajes*: estructura y funciones para guardar/mostrar scores

## 🛠️ Requisitos

- Sistema operativo **Linux / macOS** (usa `termios` y `unistd`).
- Compilador de C++ compatible con **C++11 o superior** (ej: `g++`).

## 📦 Instalación de Dependencias 

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## ▶️ Cómo compilar y ejecutar

```bash
# Compilar
g++ Galaga.cpp Pantalla.cpp Nave.cpp Enemigo.cpp -o Galaga.exe -lncurses

# Ejecutar
./Galaga.exe
```

## 📸 Vista Previa (ASCII Art)

Pantalla de inicio:
```bash
                     ██████   █████  ██      █████   ██████   █████ 
                    ██       ██   ██ ██     ██   ██ ██       ██   ██
                    ██   ███ ███████ ██     ███████ ██   ███ ███████
                    ██    ██ ██   ██ ██     ██   ██ ██    ██ ██   ██
                     ██████  ██   ██ ██████ ██   ██  ██████  ██   ██

```

## 🚧 Próximos pasos

 - Implementar lógica real de juego con enemigos que se muevan
 - Añadir niveles de dificultad
 - Implementar disparos y colisiones reale

## 👨‍💻 Autores

 - Marcelo Detlefsen - 24554
 - Julián Divas - 24687
 - Marco Díaz - 24229

 - Alejandro Jeréz - 24678
