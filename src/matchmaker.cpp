#include "../include/matchmaker.hpp"
#include <errno.h>

Matchmaker::Matchmaker(int m)
{
    maxmesas = m;
    mesasactivas = 0;
    esperando = 0;
    cerrado = false;
    ocupadas.resize(m + 1, 0);
    pthread_mutex_init(&candado, NULL);
    sem_init(&cupos, 0, maxmesas);
}

Matchmaker::~Matchmaker()
{
    sem_destroy(&cupos);
    pthread_mutex_destroy(&candado);
}

int Matchmaker::solicitarMesa()
{
    pthread_mutex_lock(&candado);
    if (cerrado == true)
    {
        pthread_mutex_unlock(&candado);
        return 0;
    }
    esperando++;
    pthread_mutex_unlock(&candado);

    int resultado;
    do
    {
        resultado = sem_wait(&cupos);
    }
    while (resultado == -1 && errno == EINTR);

    pthread_mutex_lock(&candado);
    esperando--;
    if (resultado != 0 || cerrado == true)
    {
        pthread_mutex_unlock(&candado);
        return 0;
    }

    int idmesa = 0;
    for (int i = 1; i <= maxmesas; i++)
    {
        if (ocupadas[i] == 0)
        {
            ocupadas[i] = 1;
            mesasactivas++;
            idmesa = i;
            break;
        }
    }
    pthread_mutex_unlock(&candado);
    return idmesa;
}

void Matchmaker::liberarMesa(int idmesa)
{
    pthread_mutex_lock(&candado);

    bool liberar = idmesa >= 1 && idmesa <= maxmesas;

    if (liberar)
    {
        liberar = ocupadas[idmesa] == 1;
    }

    if (liberar)
    {
        ocupadas[idmesa] = 0;
        mesasactivas--;

        if (cerrado == false)
        {
            sem_post(&cupos);
        }
    }

    pthread_mutex_unlock(&candado);
}


void Matchmaker::cerrar()
{
    pthread_mutex_lock(&candado);
    if (cerrado == true)
    {
        pthread_mutex_unlock(&candado);
        return;
    }
    cerrado = true;
    int pendientes = esperando;
    pthread_mutex_unlock(&candado);
    for (int i = 0; i < pendientes; i++)
    {
        sem_post(&cupos);
    }
}

int Matchmaker::getMesasActivas()
{
    pthread_mutex_lock(&candado);
    int resultado = mesasactivas;
    pthread_mutex_unlock(&candado);
    return resultado;
}