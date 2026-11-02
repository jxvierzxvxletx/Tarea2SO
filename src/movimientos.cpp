#include "juez.hpp"
#include "tablero.hpp"

bool Juez::movalido(Tablero &tablero, int ficha, int inicio, int final)
{
    if (inicio < 0 || inicio >= 28 || final < 0 || final >= 28 || ficha < 1 || ficha > 13)
    {
        return false;

    }

    if(tablero.consultarposicion(inicio) != ficha|| tablero.consultarposicion(final) != 0 )
    {
        return false;
    }

    if (tablero.seconectan(inicio,final) == false)
    {
        return false;
    }

    return true;
}

bool Juez::capturarperro(Tablero &tablero, int inicio, int final, int posicion_perro)
{
    if(inicio < 0 || inicio >= 28 || posicion_perro < 0 || posicion_perro >= 28 || final < 0 || final >= 28  )
    {
        return false;


    }
     // si no es el puma no se puede comer al pero //
    if (tablero.consultarposicion(inicio) != 1) 
    {
        return false;
    }

    if (tablero.consultarposicion(posicion_perro) < 2 ||  tablero.consultarposicion(posicion_perro) > 13)
    {
        return false;
    }

    if (tablero.consultarposicion(final) != 0)
    {
        return false;
    }






    // cuadrado //

    if (tablero.seconectan(inicio, posicion_perro) == false || tablero.seconectan(posicion_perro, final) == false)
    {
        return false;
    }

    if (inicio < 25 && posicion_perro < 25 && final < 25)
    {    
        return posicion_perro - inicio == final - posicion_perro;
    }


    // conexion triangulo //

    if (inicio == 25 && posicion_perro == 26 && final == 0)
    {
        return true;
    }
    if (inicio == 0 && posicion_perro == 26 && final == 25)
    {
        return true;
    }

    if (inicio == 25 && posicion_perro == 27 && final == 4)
    {
        return true;
    }
    if (inicio == 4 && posicion_perro == 27 && final == 25)
    {
        return true;
    }

    if (inicio == 25 && posicion_perro == 2 && final == 7)
    {
        return true;
    }
    if (inicio == 7 && posicion_perro == 2 && final == 25)
    {
        return true;
    }

    if (inicio == 27 && posicion_perro == 3 && final == 8)
    {
        return true;
    }
    if (inicio == 8 && posicion_perro == 3 && final == 27)
    {
        return true;
    }

    if (inicio == 26 && posicion_perro == 1 && final == 6)
    {
        return true;
    }
    if (inicio == 6 && posicion_perro == 1 && final == 26)
    {
        return true;
    }

    return false;
   

}
