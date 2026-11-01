// salaespera.cpp - [A] Sala de espera: llegada de clientes y armado de parejas (informe 3.2)
#include "salaespera.hpp"

SalaEspera::SalaEspera()
{
    llegadas = 0;
    cerrada = false;
    pthread_mutex_init(&candado, NULL);
    pthread_cond_init(&aviso, NULL);
}

SalaEspera::~SalaEspera()
{
    pthread_mutex_destroy(&candado);
    pthread_cond_destroy(&aviso);
}

// Un cliente llega: se le asigna el rol segun su orden de llegada
// (1ro puma, 2do perros, 3ro puma, ...) y entra a la cola de ese rol.
void SalaEspera::registrarLlegada(Cliente * c)
{
    pthread_mutex_lock(&candado);
    llegadas++;
    if (llegadas % 2 == 1)
    {
        c -> setRol("puma");
        colapuma.push_back(c);
    }
    else
    {
        c -> setRol("perros");
        colaperros.push_back(c);
    }
    pthread_cond_signal(&aviso);
    pthread_mutex_unlock(&candado);
}

// Espera (dormido) hasta que haya un puma y un perros esperando.
// Devuelve la pareja [puma, perros], o una lista vacia si la sala se cerro.
vector <Cliente*> SalaEspera::sacarPareja()
{
    vector <Cliente*> pareja;
    pthread_mutex_lock(&candado);
    while (cerrada == false && (colapuma.empty() || colaperros.empty()))
    {
        pthread_cond_wait(&aviso, &candado);
    }
    if (cerrada == false)
    {
        pareja.push_back(colapuma[0]);
        pareja.push_back(colaperros[0]);
        colapuma.erase(colapuma.begin());
        colaperros.erase(colaperros.begin());
    }
    pthread_mutex_unlock(&candado);
    return pareja;
}

// Cierra la sala (Ctrl+C): despierta a quien espera una pareja para que termine.
void SalaEspera::cerrar()
{
    pthread_mutex_lock(&candado);
    cerrada = true;
    pthread_cond_broadcast(&aviso);
    pthread_mutex_unlock(&candado);
}

int SalaEspera::getLlegadas()
{
    pthread_mutex_lock(&candado);
    int resultado = llegadas;
    pthread_mutex_unlock(&candado);
    return resultado;
}