#ifndef SALAESPERA_HPP
#define SALAESPERA_HPP
#include <iostream>
#include <vector>
#include <pthread.h>
#include "../include/cliente.hpp"
using namespace std;

class SalaEspera
{
    private:
        vector <Cliente*> colapuma;     // clientes Puma esperando pareja
        vector <Cliente*> colaperros;   // clientes Perros esperando pareja
        int llegadas;                   // cuantos clientes han llegado hasta ahora
        bool cerrada;                   // true cuando ya no se aceptan mas parejas (Ctrl+C)
        pthread_mutex_t candado;        // protege llegadas, cerrada y las dos colas
        pthread_cond_t aviso;           // despierta al que espera una pareja
    public:
        SalaEspera();
        ~SalaEspera();
        void registrarLlegada(Cliente * c);
        vector <Cliente*> sacarPareja();
        void cerrar();
        int getLlegadas();
};

#endif