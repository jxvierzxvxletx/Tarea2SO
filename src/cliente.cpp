// cliente.cpp - [A] Un jugador (cliente) que llega al servidor (informe 3.2)
#include "../include/cliente.hpp"

Cliente::Cliente(int id)
{
    this -> id = id;
    rol = "sinasignar";
    terminado = false;
    pthread_mutex_init(&candado, NULL);
    pthread_cond_init(&aviso, NULL);
}

Cliente::~Cliente()
{
    pthread_mutex_destroy(&candado);
    pthread_cond_destroy(&aviso);
}

bool Cliente::getTerminado()
{
    pthread_mutex_lock(&candado);
    bool resultado = terminado;
    pthread_mutex_unlock(&candado);
    return resultado;
}

// Espera pasiva: el hilo se duerme y no gasta procesador hasta que lo despierten
void Cliente::esperarFin()
{
    pthread_mutex_lock(&candado);
    while (terminado == false)
    {
        pthread_cond_wait(&aviso, &candado);
    }
    pthread_mutex_unlock(&candado);
}

void Cliente::avisarFin()
{
    pthread_mutex_lock(&candado);
    terminado = true;
    pthread_cond_broadcast(&aviso);
    pthread_mutex_unlock(&candado);
}