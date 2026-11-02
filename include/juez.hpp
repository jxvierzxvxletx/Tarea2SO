#ifndef JUEZ_HPP
#define JUEZ_HPP

#include "tablero.hpp"

class Juez
{
public:
    bool movalido(Tablero &tablero, int ficha,int inicio, int final);

    bool capturarperro(Tablero &tablero, int inicio,int final, int posicion_perro);

    bool hacermov(Tablero &tablero, int ficha, int inicio, int final);
     
};
;

#endif