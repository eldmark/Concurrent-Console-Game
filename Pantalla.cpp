#include "Pantalla.h"

/*
Universidad del Valle de Guatemala
Facultad de Ingeniera
Curso: Programación de Microprocesadores
Proyecto Final: Galaga 
Autores:
    - Marcelo Detlefsen 24553
    - Alejandro Jerez 24678
    - Julián Divas 24687
    - Marco Díaz 24229

*/

Pantalla::Pantalla(int w, int h) : ancho(w), alto(h) {
    initscr();            // iniciar ncurses
    noecho();             // no mostrar las teclas
    curs_set(0);          // ocultar cursor
    keypad(stdscr, TRUE); // habilitar teclas especiales
    nodelay(stdscr, TRUE); // input no bloqueante
    resize_term(h, w);
}

Pantalla::~Pantalla() {
    endwin(); // cerrar ncurses
}

// Limpiar pantalla
void Pantalla::limpiar() {
    clear();
}

// Dibujar caracter en posición (x, y)
void Pantalla::dibujar(int x, int y, char c) {
    if (x >= 0 && x < ancho && y >= 0 && y < alto) {
        mvaddch(y, x, c);
    }
}

// Refrescar pantalla
void Pantalla::mostrar() {
    refresh();
}

//Getters
int Pantalla::getAncho() const { return ancho; }
int Pantalla::getAlto() const { return alto; }
