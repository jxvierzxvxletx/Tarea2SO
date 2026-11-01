#ifndef TABLERO_HPP
#define TABLERO_HPP

class Tablero
{
private:
    int posiciones[28];
    bool conexiones[28][28];

public:
    Tablero();
    bool seconectan(int origen, int destino);
    int consultarposicion (int posicion);
    bool colocarficha (int posicion , int ficha);
};

#endif