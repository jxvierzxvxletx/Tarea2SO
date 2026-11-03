#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <atomic>
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
std::atomic<bool> sigintrecibido(false);
std::atomic<bool> fallovigilante(false);
static_assert(std::atomic<bool>::is_always_lock_free);
bool detenervigilante = false;
pthread_mutex_t candadovigilante;

class DatosMesaPrueba
{
    public:
        int idpartida;
        int idmesa;
        Cliente * puma;
        Cliente * perros;

        DatosMesaPrueba(int n, int mesa, Cliente * a, Cliente * b)
        {
            idpartida = n;
            idmesa = mesa;
            puma = a;
            perros = b;
        }
};

void * hiloVigilante(void * arg)
{
    sigset_t * senales = (sigset_t *)arg;
    int senal = 0;
    int resultado = sigwait(senales, &senal);

    pthread_mutex_lock(&candadovigilante);

    if (detenervigilante == true)
    {
        pthread_mutex_unlock(&candadovigilante);
        return NULL;
    }

    if (resultado != 0)
    {
        fallovigilante.store(true);
        sigintrecibido.store(true);
        sala -> cerrar();
        matchmaker -> cerrar();
        pthread_mutex_unlock(&candadovigilante);

        registro -> evento(0, "Error en sigwait. Codigo: " + to_string(resultado));
        return NULL;
    }

    sigintrecibido.store(true);
    sala -> cerrar();
    matchmaker -> cerrar();
    pthread_mutex_unlock(&candadovigilante);

    registro -> evento(0, "Se solicito cerrar el servidor");
    return NULL;
}

void * hiloCliente(void * arg)
{
    Cliente * c = (Cliente *)arg;

    pthread_mutex_lock(&candadovigilante);
    if (sigintrecibido.load() == false)
    {
        sala -> registrarLlegada(c);
    }
    pthread_mutex_unlock(&candadovigilante);

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
    registro -> evento(datos -> idmesa, "Partida " + to_string(datos -> idpartida) + " de prueba: cliente " + to_string(datos -> puma -> getId()) + " (puma) y cliente " + to_string(datos -> perros -> getId()) + " (perros). Mesas activas: " + to_string(matchmaker -> getMesasActivas()));

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
        registro -> evento(datos -> idmesa, "Partida " + to_string(datos -> idpartida) + " de prueba interrumpida");
    }
    else
    {
        registro -> evento(datos -> idmesa, "Partida " + to_string(datos -> idpartida) + " de prueba terminada");
    }

    datos -> puma -> avisarFin();
    datos -> perros -> avisarFin();
    matchmaker -> liberarMesa(datos -> idmesa);
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
    
    sigset_t senales;
    sigemptyset(&senales);
    sigaddset(&senales, SIGINT);

    if (pthread_sigmask(SIG_BLOCK, &senales, NULL) != 0)
    {
        registro -> evento(0, "Error al bloquear SIGINT");
        delete matchmaker;
        delete sala;
        delete registro;
        delete cfg;
        delete args;
        pthread_mutex_destroy(&candadovigilante);
        return 1;
    }

    pthread_t vigilante;
    if (pthread_create(&vigilante, NULL, hiloVigilante, &senales) != 0)
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
        pthread_mutex_lock(&candadovigilante);

    if (sigintrecibido.load() == true)
    {
        pthread_mutex_unlock(&candadovigilante);
        break;
    }

    Cliente * c = new Cliente(i + 1);
    pthread_t hilo;
    int resultado = pthread_create(&hilo, NULL, hiloCliente, c);

    pthread_mutex_unlock(&candadovigilante);

    if (resultado != 0)
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
        if (sigintrecibido.load())
        {
            break;
        }
        vector <Cliente*> pareja = sala -> sacarPareja();
        if (pareja.empty())
        {
            break;
        }
        int idmesa = matchmaker -> solicitarMesa();
        if (idmesa == 0)
        {
            pareja[0] -> avisarFin();
            pareja[1] -> avisarFin();
            break;
        }
        
    pthread_mutex_lock(&candadovigilante);

    if (sigintrecibido.load() == true)
    {
        pthread_mutex_unlock(&candadovigilante);
        matchmaker -> liberarMesa(idmesa);
        pareja[0] -> avisarFin();
        pareja[1] -> avisarFin();
        break;
    }

    DatosMesaPrueba * datos = new DatosMesaPrueba(p, idmesa, pareja[0], pareja[1]);
    pthread_t hilo;
    int resultado = pthread_create(&hilo, NULL, hiloMesaPrueba, datos);

    pthread_mutex_unlock(&candadovigilante);

    if (resultado != 0)
    {
        registro -> evento(0, "Error al crear hilo de mesa de prueba");
        delete datos;
        matchmaker -> liberarMesa(idmesa);
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

    if (sigintrecibido.load() == false)
    {
        pthread_kill(vigilante, SIGINT);
    }

    pthread_join(vigilante, NULL);

    if (error == true || fallovigilante.load() == true)
    {
        registro -> evento(0, "Prueba finalizada con errores, partidas iniciadas: " + to_string(hilosmesas.size()));
    }
    else if (sigintrecibido != 0)
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
    return (error || fallovigilante.load()) ? 1 : 0;
}