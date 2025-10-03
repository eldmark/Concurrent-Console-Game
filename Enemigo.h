#ifndef ENEMIGO_H
#define ENEMIGO_H

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

#include <ncurses.h>
#include "Pantalla.h"


//Clase enemigo
class Enemigo {
private:
    int xpos;
    int ypos;

//Metodos de la clase enemigo para manejjar movimiento y dibujo
public:
    Enemigo(int startX, int startY);
    void moveDown(int limite);
    void moveUp(int limite);
    void moveLeft(int limite);
    void moveRight(int limite);
    void draw(Pantalla& p);
    int getX() const;
    int getY() const;

};

#endif