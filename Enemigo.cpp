#include "Enemigo.h"
#include "Pantalla.h"

Enemigo::Enemigo(int startX, int startY) : xpos(startX), ypos(startY) {}
// controles de movimiento
void Enemigo::moveDown(int pasos) {
    ypos += pasos;
    if (ypos > 23) ypos = 23; 
}

// Limitar movimiento hacia arriba
void Enemigo::moveUp(int limite) {
    if (ypos > 0) ypos--;
}
// Limitar movimiento hacia la izquierda
void Enemigo::moveLeft(int pasos) {
    xpos -= pasos;
    if (xpos < 1) xpos = 1;
}
// Limitar movimiento hacia la derecha
void Enemigo::moveRight(int pasos) {
    xpos += pasos;
    if (xpos > 78) xpos = 78;
}

// Dibujar enemigo en pantalla
void Enemigo::draw(Pantalla& p) {
    p.dibujar(xpos, ypos, 'X');
}
int Enemigo::getX() const { return xpos; }
int Enemigo::getY() const { return ypos; }

void avoidCollision(Enemigo& e1, Enemigo& e2) {
    if (e1.getX() == e2.getX() && e1.getY() == e2.getY()) {
        e1.moveLeft(25);
    }
}