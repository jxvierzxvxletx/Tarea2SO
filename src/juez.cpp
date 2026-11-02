#include "tablero.hpp"
#include "movimientos.hpp"

bool hacermov(Tablero &tablero, int ficha, int inicio, int final)
{
    if (Movimientos.movalidos(Tablero &tablero, ficha, inicio, final) == false)
    {
        return false;
    }

    bool resultado = tablero.moverlaficha(inicio, final); // deja inicio vacia//

    return resultado;


}