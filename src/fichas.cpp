
#include "mesa.hpp"
#include <array>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
using namespace std;
using namespace chrono;
using namespace this_thread;

class datosficha
{
public:
    Mesa *mesa;
    int ficha;
    int retardomin;
    int retardomax;
    unsigned int semilla;
};




void *hiloficha(void *argumento)
{
    datosficha *datos = static_cast<datosficha *>(argumento);
    Mesa *mesa = datos->mesa;
    int ficha = datos->ficha;

    minstd_rand generador(datos->semilla);
    uniform_int_distribution<int> retardo(datos->retardomin,datos->retardomax);

    Tablero copia;
    Juez reglas;

    while (true)
    {
        int espera = retardo(generador);
        sleep_for(milliseconds(espera));

        if (mesa->copiartablero(copia) == false)
        {
            return nullptr;

        }
    

        int inicio = -1;

        for (int i = 0; i < 28; i ++)
        {
            if (copia.consultarposicion(i) == ficha)
            {
                inicio = i;
                break;
            }
        }

        if (inicio == -1)
        {
            return nullptr;
        }

        vector<array<int, 3>> jugadas;
        for (int i = 0; i < 28; i ++)
        {
            if (reglas.movalido(copia, ficha, inicio, i))
            {
                jugadas.push_back({i, 0, 0});
            }
        }
        if (ficha == 1)
        {
            for (int i = 0; i < 28; i ++) //perro
            {
                for (int j = 0; j < 28; j++) //final
                {

                    if (reglas.capturarperrovalido(copia, inicio, j, i))
                    {
                        
                        jugadas.push_back({j, 1, i});
                    }
                }
            }
        }
        if (jugadas.empty())
        {
            continue;
        }

        uniform_int_distribution<int> elegir(
            0, static_cast<int>(jugadas.size()) - 1
        );

        array<int, 3> jugada = jugadas[elegir(generador)];

        if (mesa->f_quierejugar(ficha, inicio, jugada[0],jugada[1] == 1, jugada[2]) == false)
        {
            return nullptr;
        }
    }
}


bool jugarpartida(Mesa &mesa, int retardomin, int retardomax, unsigned int semilla)
{
    pthread_t juez_hilo;
    pthread_t hilos[13];
    datosficha argumentos[13];
    int creados = 0;

    if (pthread_create(&juez_hilo, nullptr, hiloJuez, &mesa) != 0)
    {
        mesa.mesacerrada();
        return false;
    }

    for (int i = 0; i < 13; i ++)
    {
        argumentos[i].mesa = &mesa;
        argumentos[i].ficha = i + 1;
        argumentos[i].retardomin = retardomin;
        argumentos[i].retardomax = retardomax;
         argumentos[i].semilla = semilla + i;

        if (pthread_create(&hilos[i], nullptr, hiloficha, &argumentos[i]) != 0)
        {
            mesa.mesacerrada();
            break;
        }

        creados++;
    }

    pthread_join(juez_hilo, nullptr);

    for (int i = 0; i < creados; i ++)
    {
        pthread_join(hilos[i], nullptr);
    }

    return creados == 13;
}