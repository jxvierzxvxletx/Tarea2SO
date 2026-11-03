#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "../include/argumentos.hpp"
#include "../include/cfg.hpp"
#include "../include/cliente.hpp"
#include "../include/salaespera.hpp"
#include "../include/logs.hpp"
#include "../include/matchmaker.hpp"
using namespace std;

Cfg * cfg;
Registro * registro;
SalaEspera * sala;
Matchmaker * matchmaker;
volatile sig_atomic_t sigintrecibido = 0;
bool detenervigilante = false;
pthread_mutex_t candadovigilante;

class DatosMesaPrueba
{
    public:
        int id;
        Cliente * puma;
        Cliente * perros;

        DatosMesaPrueba(int n, Cliente * a, Cliente * b)
        {
            id = n;
            puma = a;
            perros = b;
        }
};

void recibirSigint(int senal)
{
    (void)senal;
    sigintrecibido = 1;
}

void * hiloVigilante(void * arg)
{
    (void)arg;
    while (true)
    {
        if (sigintrecibido != 0)
        {
            sala -> cerrar();
            matchmaker -> cerrar();
            registro -> evento(0, "Se solicito cerrar el servidor");
            return NULL;
        }

        pthread_mutex_lock(&candadovigilante);
        bool terminar = detenervigilante;
        pthread_mutex_unlock(&candadovigilante);
        if (terminar)
        {
            return NULL;
        }
        usleep(10000);
    }
}

void * hiloCliente(void * arg)
{
    Cliente * c = (Cliente *)arg;
    sala -> registrarLlegada(c);
    if (c -> getRol() == "sinasignar")
    {
        return NULL;
    }

    registro -> evento(0, "Cliente " + to_string(c -> getId()) + " llego como " + c -> getRol());
    c -> esperarFin();
    registro -> evento(0, "Cliente " + to_string(c -> getId()) + " termino su espera");
    return NULL;
}

void * hiloMesaPrueba(void * arg)
{
    DatosMesaPrueba * datos = (DatosMesaPrueba *)arg;
    registro -> evento(datos -> id, "Partida de prueba: cliente " + to_string(datos -> puma -> getId()) + " (puma) y cliente " + to_string(datos -> perros -> getId()) + " (perros). Mesas activas: " + to_string(matchmaker -> getMesasActivas()));

    for (int i = 0; i < 10; i++)
    {
        if (sigintrecibido != 0)
        {
            break;
        }
        usleep(50000);
    }

    if (sigintrecibido != 0)
    {
        registro -> evento(datos -> id, "Partida de prueba interrumpida");
    }
    else
    {
        registro -> evento(datos -> id, "Partida de prueba terminada");
    }

    datos -> puma -> avisarFin();
    datos -> perros -> avisarFin();
    matchmaker -> liberarMesa();
    delete datos;
    return NULL;
}

int main(int argc, char * argv[])
{
    Argumentos * args = new Argumentos();
    if (args -> leer(argc, argv) == false)
    {
        delete args;
        return 1;
    }

    cfg = new Cfg();
    if (args -> getHayConfig() == true && cfg -> cargarConfig(args -> getRutaConfig()) == false)
    {
        delete cfg;
        delete args;
        return 1;
    }

    int n = args -> getJugadores();
    int m = args -> getMesas();
    registro = new Registro(cfg);
    sala = new SalaEspera();
    matchmaker = new Matchmaker(m);
    pthread_mutex_init(&candadovigilante, NULL);

    if (signal(SIGINT, recibirSigint) == SIG_ERR)
    {
        registro -> evento(0, "Error al instalar el manejador de Ctrl+C");
        delete matchmaker;
        delete sala;
        delete registro;
        delete cfg;
        delete args;
        pthread_mutex_destroy(&candadovigilante);
        return 1;
    }

    pthread_t vigilante;
    if (pthread_create(&vigilante, NULL, hiloVigilante, NULL) != 0)
    {
        registro -> evento(0, "Error al crear hilo vigilante");
        delete matchmaker;
        delete sala;
        delete registro;
        delete cfg;
        delete args;
        pthread_mutex_destroy(&candadovigilante);
        return 1;
    }

    registro -> evento(0, "Servidor iniciado con " + to_string(n) + " clientes y maximo " + to_string(m) + " mesas");

    vector <Cliente*> clientes;
    vector <pthread_t> hilosclientes;
    bool error = false;
    for (int i = 0; i < n; i++)
    {
        if (sigintrecibido != 0)
        {
            break;
        }
        Cliente * c = new Cliente(i + 1);
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, hiloCliente, c) != 0)
        {
            delete c;
            registro -> evento(0, "Error al crear hilo cliente");
            sala -> cerrar();
            matchmaker -> cerrar();
            error = true;
            break;
        }
        clientes.push_back(c);
        hilosclientes.push_back(hilo);
    }

    vector <pthread_t> hilosmesas;
    int maxpartidas = hilosclientes.size() / 2;
    for (int p = 1; p <= maxpartidas && error == false; p++)
    {
        if (sigintrecibido != 0)
        {
            break;
        }
        vector <Cliente*> pareja = sala -> sacarPareja();
        if (pareja.empty())
        {
            break;
        }
        if (matchmaker -> solicitarMesa() == false)
        {
            pareja[0] -> avisarFin();
            pareja[1] -> avisarFin();
            break;
        }
        if (sigintrecibido != 0)
        {
            matchmaker -> liberarMesa();
            pareja[0] -> avisarFin();
            pareja[1] -> avisarFin();
            break;
        }

        DatosMesaPrueba * datos = new DatosMesaPrueba(p, pareja[0], pareja[1]);
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, hiloMesaPrueba, datos) != 0)
        {
            registro -> evento(0, "Error al crear hilo de mesa de prueba");
            delete datos;
            matchmaker -> liberarMesa();
            pareja[0] -> avisarFin();
            pareja[1] -> avisarFin();
            error = true;
            break;
        }
        hilosmesas.push_back(hilo);
    }

    for (size_t i = 0; i < hilosmesas.size(); i++)
    {
        pthread_join(hilosmesas[i], NULL);
    }

    sala -> cerrar();
    matchmaker -> cerrar();
    for (size_t i = 0; i < clientes.size(); i++)
    {
        clientes[i] -> avisarFin();
    }
    for (size_t i = 0; i < hilosclientes.size(); i++)
    {
        pthread_join(hilosclientes[i], NULL);
    }

    pthread_mutex_lock(&candadovigilante);
    detenervigilante = true;
    pthread_mutex_unlock(&candadovigilante);
    pthread_join(vigilante, NULL);

    if (sigintrecibido != 0)
    {
        registro -> evento(0, "Prueba interrumpida, partidas iniciadas: " + to_string(hilosmesas.size()));
    }
    else
    {
        registro -> evento(0, "Prueba finalizada, partidas iniciadas: " + to_string(hilosmesas.size()));
    }
    for (size_t i = 0; i < clientes.size(); i++)
    {
        delete clientes[i];
    }

    delete matchmaker;
    delete sala;
    delete registro;
    delete cfg;
    delete args;
    pthread_mutex_destroy(&candadovigilante);
    return error ? 1 : 0;
}