#ifndef SALAESPERA_HPP
#define SALAESPERA_HPP
#include <iostream>
#include <vector>
#include <pthread.h>
#include "cliente.hpp"
using namespace std;

class SalaEspera
{
    private:
        vector <Cliente*> colapuma;     
        vector <Cliente*> colaperros;   
        int llegadas;                   
        bool cerrada;                   
        pthread_mutex_t candado;        
        pthread_cond_t aviso;           
    public:
        SalaEspera();
        ~SalaEspera();
        void registrarLlegada(Cliente * c);
        vector <Cliente*> sacarPareja();
        void cerrar();
        int getLlegadas();
};

#endif