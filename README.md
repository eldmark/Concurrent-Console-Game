## GALAGA – Concurrent Console Game in C++

A **console-based game implemented in C++**, inspired by the classic *Galaga*, featuring real-time gameplay, enemy movement, collision detection, and coordinated boss behavior. The game uses **ncurses** for terminal rendering and **pthreads** for concurrency and synchronization.

---

##  Project Overview

This project implements a functional **console game engine** running entirely in the terminal. It includes a real-time game loop, player input handling, enemy AI, collision detection, and a multi-threaded boss system with coordinated attacks.

The goal of the project is to explore **low-level game logic**, **concurrent programming**, and **state management** using C++ in a terminal-based environment.

---

## Key Technical Concepts

- Real-time game loop
- Multi-threaded enemy and boss behavior
- Synchronization using mutexes and condition variables
- Collision detection
- State management (player, enemies, boss, shots)
- Terminal-based rendering using ncurses

---

##  Features

- Interactive terminal UI with ASCII graphics and ANSI colors
- Splash screen with ASCII art
- Player-controlled ship with movement and actions
- Real enemy movement and collision logic
- Boss system with:
  - Health bar
  - Escort enemies
  - Coordinated attack patterns
  - Independent threads for behavior execution
- Shooting mechanics for player, enemies, and boss
- Score system with ranking
- Score persistence using CSV files

---

## Project Structure
```
Galaga.cpp # Main game loop and core logic
Pantalla.h / .cpp # Terminal rendering abstraction (ncurses)
Nave.h / .cpp # Player ship logic
Enemigo.h / .cpp # Enemy behavior and movement
Boss.h / .cpp # Boss logic, escorts, and coordinated attacks
```


The codebase is modular, with clear separation of responsibilities between rendering, input, game entities, and concurrency control.

---

##  Technologies Used

- C++ (C++11 or higher)
- ncurses (terminal graphics)
- pthreads (multithreading and synchronization)
- Linux / macOS system libraries (`termios`, `unistd`)

---

##  Requirements

- Linux or macOS
- C++ compiler supporting C++11+
- ncurses development libraries
- pthread support

---

### Dependency Installation (Linux)

```bash
    sudo apt-get install libncurses5-dev libncursesw5-dev mpg123
```
## Build and Run
  ```bash

    g++ Galaga.cpp Pantalla.cpp Nave.cpp Enemigo.cpp Boss.cpp -o Galaga -lncurses -lpthread
  ```
## Run
./Galaga

### Gameplay Overview

Player controls the ship using keyboard input

Enemies move dynamically and interact with the player

Collisions are detected between shots, enemies, and the player

Boss enemies execute coordinated attacks using multiple threads

Scores are recorded and ranked during gameplay

### Preview (ASCII Art)
```bash
 ██████   █████  ██      █████   ██████   █████
██       ██   ██ ██     ██   ██ ██       ██   ██
██   ███ ███████ ██     ███████ ██   ███ ███████
██    ██ ██   ██ ██     ██   ██ ██    ██ ██   ██
 ██████  ██   ██ ██████ ██   ██  ██████  ██   ██
```

### Future Improvements
Additional enemy movement patterns

Difficulty levels

Enhanced collision handling and effects

Expanded boss behaviors and attack strategies

Improved persistence and statistics tracking

### Authors
Marcelo Detlefsen

Julián Divas

Marco Díaz
- Thread logic to enemies move
- Sincronizing movement of the enemies
- Programing the movement of the main starship

Alejandro Jeréz


