#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <pthread.h>
#include <semaphore.h>
#include <vector>
using namespace std;

class Matchmaker
{
    private:
        int maxmesas;
        int mesasactivas;
        int esperando;
        bool cerrado;
        vector <int> ocupadas;
        sem_t cupos;
        pthread_mutex_t candado;

    public:
        Matchmaker(int m);
        ~Matchmaker();
        int solicitarMesa();
        void liberarMesa(int idmesa);
        void cerrar();
        int getMesasActivas();
};

#endif