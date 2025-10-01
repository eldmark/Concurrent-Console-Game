#ifndef NAVE_H
#define NAVE_H

#include "Pantalla.h"

class Nave {
private:
    int x, y;

public:
    Nave(int startX, int startY);
    void moverIzquierda();
    void moverDerecha(int limite);
    void dibujar(Pantalla& p);
    int getX() const;
    int getY() const;
};
#endif

