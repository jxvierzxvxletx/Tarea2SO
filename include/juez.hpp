#ifndef JUEZ_HPP
#define JUEZ_HPP

#include "tablero.hpp"

class Juez

{
private:

int cont = 0;


public:
    bool movalido(Tablero &tablero, int ficha,int inicio, int final);

    bool capturarperrovalido(Tablero &tablero, int inicio,int final, int posicion_perro);

    bool hacermov(Tablero &tablero, int ficha, int inicio, int final);

    bool capturar(Tablero &tablero, int inicio, int posicion_perro, int final);

    bool winpuma();

    bool loosepuma(Tablero &tablero);
     
}
;

#endif