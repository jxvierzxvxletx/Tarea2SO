#ifndef TABLERO_HPP
#define TABLERO_HPP
#include "cfg.hpp"

class Tablero
{
private:
    int posiciones[28];
    bool conexiones[28][28];

public:
    Tablero();
    bool seconectan(int inicio, int final);
    int consultarposicion (int posicion);
    bool colocarficha (int posicion , int ficha);
    bool moverlaficha (int inicio, int final);
    bool fichamuerta (int posicion);
    bool feet(Cfg &datos );
    bool tvacio();
};

#endif