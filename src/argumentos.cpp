#include "argumentos.hpp"

Argumentos::Argumentos()
{
    jugadores = 0;
    mesas = 0;
    rutaconfig = "";
    hayconfig = false;
}

int Argumentos::convertirNumero(string texto)
{
    int tam = texto.size();
    if (tam == 0 || tam > 6)
    {
        return -1;
    }
    int numero = 0;
    for (int i = 0; i < tam; i++)
    {
        if (texto[i] < '0' || texto[i] > '9')
        {
            return -1;
        }
        numero = numero * 10 + (texto[i] - '0');
    }
    return numero;
}

bool Argumentos::leer(int argc, char * argv[])
{
    if (argc != 3 && argc != 4)
    {
        cout << "Uso: ./torneo N M [config.txt]" << endl;
        return false;
    }
    jugadores = convertirNumero(argv[1]);
    if (jugadores < 2 || jugadores % 2 != 0)
    {
        cout << "Error: N debe ser un numero entero par, mayor o igual a 2 (los jugadores se emparejan de a 2)" << endl;
        return false;
    }
    mesas = convertirNumero(argv[2]);
    if (mesas < 1)
    {
        cout << "Error: M debe ser un numero entero mayor o igual a 1" << endl;
        return false;
    }
    if (argc == 4)
    {
        rutaconfig = argv[3];
        hayconfig = true;
    }
    return true;
}