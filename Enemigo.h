#ifndef ENEMIGO_H
#define ENEMIGO_H

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