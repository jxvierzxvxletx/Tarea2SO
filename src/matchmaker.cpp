#include "../include/matchmaker.hpp"
#include <errno.h>

Matchmaker::Matchmaker(int m)
{
    maxmesas = m;
    mesasactivas = 0;
    esperando = 0;
    cerrado = false;
    pthread_mutex_init(&candado, NULL);
    sem_init(&cupos, 0, maxmesas);
}

Matchmaker::~Matchmaker()
{
    sem_destroy(&cupos);
    pthread_mutex_destroy(&candado);
}

bool Matchmaker::solicitarMesa()
{
    pthread_mutex_lock(&candado);
    if (cerrado == true)
    {
        pthread_mutex_unlock(&candado);
        return false;
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
    bool permitido = resultado == 0 && cerrado == false;
    if (permitido)
    {
        mesasactivas++;
    }
    pthread_mutex_unlock(&candado);
    return permitido;
}

void Matchmaker::liberarMesa()
{
    pthread_mutex_lock(&candado);
    bool liberar = mesasactivas > 0;
    if (liberar)
    {
        mesasactivas--;
    }
    bool avisar = liberar && cerrado == false;
    pthread_mutex_unlock(&candado);
    if (avisar)
    {
        sem_post(&cupos);
    }
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