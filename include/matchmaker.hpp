#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <pthread.h>
#include <semaphore.h>

class Matchmaker
{
    private:
        int maxmesas;
        int mesasactivas;
        int esperando;
        bool cerrado;
        sem_t cupos;
        pthread_mutex_t candado;

    public:
        Matchmaker(int m);
        ~Matchmaker();
        bool solicitarMesa();
        void liberarMesa();
        void cerrar();
        int getMesasActivas();
};

#endif