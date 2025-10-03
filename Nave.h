#ifndef NAVE_H
#define NAVE_H

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

