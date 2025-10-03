#ifndef PANTALLA_H
#define PANTALLA_H

#include <ncurses.h>

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

//Clase pantalla para manejar la interfaz visual
class Pantalla {
private:
    int ancho;
    int alto;

public:
    Pantalla(int w, int h);
    ~Pantalla();
// Métodos para limpiar, dibujar y mostrar en pantalla
    void limpiar();
    void dibujar(int x, int y, char c);
    void mostrar();
    int getAncho() const;
    int getAlto() const;
};

#endif
