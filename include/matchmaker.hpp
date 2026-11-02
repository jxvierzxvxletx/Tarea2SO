#ifndef MATCHMAKER_HPP
#define MATCHMAKER_HPP

#include <pthread.h>

class Matchmaker
{
    private:
        int maxmesas;
        int mesasactivas;
        bool cerrado;
        pthread_mutex_t candado;
        pthread_cond_t aviso;

    public:
        Matchmaker(int maxmesas);
        ~Matchmaker();
        bool solicitarMesa();
        void liberarMesa();
        void cerrar();
        int getMesasActivas();
};

#endif