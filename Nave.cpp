#include "Nave.h"

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

//Métodos de la clase nave para manejar movimiento y dibujo
Nave::Nave(int startX, int startY) : x(startX), y(startY) {}

void Nave::moverIzquierda() {
    if (x > 0) x--;
}

void Nave::moverDerecha(int limite) {
    if (x < limite - 1) x++;
}

void Nave::dibujar(Pantalla& p) {
    p.dibujar(x, y, 'A'); // símbolo de la nave
}

int Nave::getX() const { return x; }
int Nave::getY() const { return y; }
