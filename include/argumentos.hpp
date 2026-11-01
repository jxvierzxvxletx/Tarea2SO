// argumentos.hpp - [A] Lectura de ./torneo N M [config.txt] (informe 3)
#ifndef ARGUMENTOS_HPP
#define ARGUMENTOS_HPP
#include <iostream>
#include <string>
using namespace std;

class Argumentos
{
    private:
        int jugadores;        // N: total de clientes (siempre par)
        int mesas;            // M: maximo de mesas simultaneas
        string rutaconfig;    // ruta del config.txt (solo si hayconfig es true)
        bool hayconfig;
        int convertirNumero(string texto);
    public:
        Argumentos();
        bool leer(int argc, char * argv[]);
        int getJugadores()
        {
            return jugadores;
        }
        int getMesas()
        {
            return mesas;
        }
        string getRutaConfig()
        {
            return rutaconfig;
        }
        bool getHayConfig()
        {
            return hayconfig;
        }
};

#endif