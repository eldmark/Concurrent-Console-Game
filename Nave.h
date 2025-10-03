#ifndef NAVE_H
#define NAVE_H

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

#include "Pantalla.h"
//Clase nave para manejar la nave del jugador
class Nave {
private:
    int x, y;

    //Declaración de métodos para mover y dibujo de la nave
public:
    Nave(int startX, int startY);
    void moverIzquierda();
    void moverDerecha(int limite);
    void dibujar(Pantalla& p);
    int getX() const;
    int getY() const;
};
#endif

