#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <string>
#include <vector>
#include "../include/argumentos.hpp"
#include "../include/cfg.hpp"
#include "../include/cliente.hpp"
#include "../include/logs.hpp"
#include "../include/matchmaker.hpp"
#include "../include/mesa.hpp"
#include "../include/salaespera.hpp"
using namespace std;

Cfg *cfg;
Registro *registro;
SalaEspera *sala;
Matchmaker *matchmaker;

atomic<bool> sigintrecibido(false);
atomic<bool> fallovigilante(false);
bool detenervigilante = false;

pthread_mutex_t candadovigilante;
vector<Mesa *> mesasactivas;

class DatosPartida
{
public:
    int idpartida;
    int idmesa;
    Cliente *puma;
    Cliente *perros;

    DatosPartida(int n, int mesa, Cliente *a, Cliente *b)
    {
        idpartida = n;
        idmesa = mesa;
        puma = a;
        perros = b;
    }
};

class DatosAbandonoPrueba
{
public:
    Mesa *mesa;
    string rol;
};

void *hiloAbandonoPrueba(void *arg)
{
    DatosAbandonoPrueba *datos = static_cast<DatosAbandonoPrueba *>(arg);
    usleep(10000);
    datos->mesa->notificarAbandono(datos->rol);
    return nullptr;
}

void *hiloVigilante(void *arg)
{
    sigset_t *senales = static_cast<sigset_t *>(arg);
    int senal = 0;
    int resultado = sigwait(senales, &senal);

    pthread_mutex_lock(&candadovigilante);

    if (detenervigilante)
    {
        pthread_mutex_unlock(&candadovigilante);
        return nullptr;
    }

    if (resultado != 0)
    {
        fallovigilante.store(true);
    }

    sigintrecibido.store(true);
    sala->cerrar();
    matchmaker->cerrar();

    for (Mesa *mesa : mesasactivas)
    {
        mesa->mesacerrada();
    }

    pthread_mutex_unlock(&candadovigilante);

    if (resultado != 0)
    {
        registro->evento(0, "Error en sigwait. Codigo: " + to_string(resultado));
    }
    else
    {
        registro->evento(0, "Se solicito cerrar el servidor");
    }

    return nullptr;
}

void *hiloCliente(void *arg)
{
    Cliente *cliente = static_cast<Cliente *>(arg);

    pthread_mutex_lock(&candadovigilante);

    if (!sigintrecibido.load())
    {
        sala->registrarLlegada(cliente);
    }

    pthread_mutex_unlock(&candadovigilante);

    if (cliente->getRol() == "sinasignar")
    {
        return nullptr;
    }

    registro->evento(
        0,
        "Cliente " + to_string(cliente->getId()) +
        " llego como " + cliente->getRol()
    );

    cliente->esperarFin();

    registro->evento(
        0,
        "Cliente " + to_string(cliente->getId()) + " termino su espera"
    );

    return nullptr;
}

void *hiloMesa(void *arg)
{
    DatosPartida *datos = static_cast<DatosPartida *>(arg);
    Mesa mesa;
    mesa.configurarRegistro(registro, datos->idmesa, datos->idpartida);
    registro->evento(
        datos->idmesa,
        "Partida " + to_string(datos->idpartida) + " asignada: cliente " +
        to_string(datos->puma->getId()) + " (puma) y cliente " +
        to_string(datos->perros->getId()) + " (perros)"
    );

    if (!mesa.iniciar(*cfg))
    {
        registro->evento(datos->idmesa, "Error al preparar el tablero");
    }
    else
    {
        pthread_mutex_lock(&candadovigilante);

        bool iniciar_partida = !sigintrecibido.load();

        if (iniciar_partida)
        {
            mesasactivas.push_back(&mesa);
        }

        pthread_mutex_unlock(&candadovigilante);

        if (iniciar_partida)
        {
            const char *pruebaPartida = getenv("KOMIKAN_ABANDONO_PARTIDA");
            const char *pruebaRol = getenv("KOMIKAN_ABANDONO_ROL");
            DatosAbandonoPrueba datosAbandono;
            pthread_t hiloAbandono;
            bool abandonoCreado = false;

            if (pruebaPartida != nullptr && pruebaRol != nullptr &&
                to_string(datos->idpartida) == pruebaPartida &&
                (string(pruebaRol) == "puma" || string(pruebaRol) == "perros"))
            {
                datosAbandono.mesa = &mesa;
                datosAbandono.rol = pruebaRol;
                if (pthread_create(&hiloAbandono, nullptr, hiloAbandonoPrueba, &datosAbandono) == 0)
                {
                    abandonoCreado = true;
                }
                else
                {
                    registro->evento(datos->idmesa, "Error al crear la prueba de abandono");
                }
            }

            bool hilos_creados = jugarpartida(
                mesa,
                cfg->getRetardoMin(),
                cfg->getRetardoMax(),
                cfg->getSemilla()
            );

            if (abandonoCreado)
            {
                pthread_join(hiloAbandono, nullptr);
            }

            pthread_mutex_lock(&candadovigilante);

            auto posicion = find(
                mesasactivas.begin(), mesasactivas.end(), &mesa
            );

            if (posicion != mesasactivas.end())
            {
                mesasactivas.erase(posicion);
            }

            pthread_mutex_unlock(&candadovigilante);

            if (!hilos_creados)
            {
                registro->evento(
                    datos->idmesa, "Error al crear los hilos de la partida"
                );
            }
            else
            {
                registro->evento(datos->idmesa, "Partida finalizada");
            }
        }
        else
        {
            registro->evento(datos->idmesa, "Partida cancelada antes de iniciar");
        }
    }

    datos->puma->avisarFin();
    datos->perros->avisarFin();
    matchmaker->liberarMesa(datos->idmesa);
    delete datos;

    return nullptr;
}

int main(int argc, char *argv[])
{
    Argumentos *args = new Argumentos();

    if (!args->leer(argc, argv))
    {
        delete args;
        return 1;
    }

    cfg = new Cfg();

    if (args->getHayConfig() && !cfg->cargarConfig(args->getRutaConfig()))
    {
        delete cfg;
        delete args;
        return 1;
    }

    int n = args->getJugadores();
    int m = args->getMesas();

    registro = new Registro(cfg);
    sala = new SalaEspera();
    matchmaker = new Matchmaker(m);
    mesasactivas.reserve(m);
    pthread_mutex_init(&candadovigilante, nullptr);

    sigset_t senales;
    sigemptyset(&senales);
    sigaddset(&senales, SIGINT);

    if (pthread_sigmask(SIG_BLOCK, &senales, nullptr) != 0)
    {
        registro->evento(0, "Error al bloquear SIGINT");
        delete matchmaker;
        delete sala;
        delete registro;
        delete cfg;
        delete args;
        pthread_mutex_destroy(&candadovigilante);
        return 1;
    }

    pthread_t vigilante;

    if (pthread_create(&vigilante, nullptr, hiloVigilante, &senales) != 0)
    {
        registro->evento(0, "Error al crear hilo vigilante");
        delete matchmaker;
        delete sala;
        delete registro;
        delete cfg;
        delete args;
        pthread_mutex_destroy(&candadovigilante);
        return 1;
    }

    registro->evento(
        0,
        "Servidor iniciado con " + to_string(n) +
        " clientes y maximo " + to_string(m) + " mesas"
    );

    vector<Cliente *> clientes;
    vector<pthread_t> hilosclientes;
    bool error = false;

    for (int i = 0; i < n; i++)
    {
        pthread_mutex_lock(&candadovigilante);

        if (sigintrecibido.load())
        {
            pthread_mutex_unlock(&candadovigilante);
            break;
        }

        Cliente *cliente = new Cliente(i + 1);
        pthread_t hilo;
        int resultado = pthread_create(&hilo, nullptr, hiloCliente, cliente);

        pthread_mutex_unlock(&candadovigilante);

        if (resultado != 0)
        {
            delete cliente;
            registro->evento(0, "Error al crear hilo cliente");
            sala->cerrar();
            matchmaker->cerrar();
            error = true;
            break;
        }

        clientes.push_back(cliente);
        hilosclientes.push_back(hilo);
    }

    vector<pthread_t> hilosmesas;
    int maxpartidas = static_cast<int>(hilosclientes.size() / 2);

    for (int p = 1; p <= maxpartidas && !error; p++)
    {
        if (sigintrecibido.load())
        {
            break;
        }

        vector<Cliente *> pareja = sala->sacarPareja();

        if (pareja.empty())
        {
            break;
        }

        int idmesa = matchmaker->solicitarMesa();

        if (idmesa == 0)
        {
            pareja[0]->avisarFin();
            pareja[1]->avisarFin();
            break;
        }

        pthread_mutex_lock(&candadovigilante);

        if (sigintrecibido.load())
        {
            pthread_mutex_unlock(&candadovigilante);
            matchmaker->liberarMesa(idmesa);
            pareja[0]->avisarFin();
            pareja[1]->avisarFin();
            break;
        }

        DatosPartida *datos = new DatosPartida(
            p, idmesa, pareja[0], pareja[1]
        );

        pthread_t hilo;
        int resultado = pthread_create(&hilo, nullptr, hiloMesa, datos);

        pthread_mutex_unlock(&candadovigilante);

        if (resultado != 0)
        {
            registro->evento(0, "Error al crear hilo de mesa");
            delete datos;
            matchmaker->liberarMesa(idmesa);
            pareja[0]->avisarFin();
            pareja[1]->avisarFin();
            error = true;
            break;
        }

        hilosmesas.push_back(hilo);
    }

    for (pthread_t hilo : hilosmesas)
    {
        pthread_join(hilo, nullptr);
    }

    sala->cerrar();
    matchmaker->cerrar();

    for (Cliente *cliente : clientes)
    {
        cliente->avisarFin();
    }

    for (pthread_t hilo : hilosclientes)
    {
        pthread_join(hilo, nullptr);
    }

    pthread_mutex_lock(&candadovigilante);
    detenervigilante = true;
    pthread_mutex_unlock(&candadovigilante);

    if (!sigintrecibido.load())
    {
        pthread_kill(vigilante, SIGINT);
    }

    pthread_join(vigilante, nullptr);

    string partidas = to_string(hilosmesas.size());

    if (error || fallovigilante.load())
    {
        registro->evento(0, "Servidor finalizado con errores. Partidas: " + partidas);
    }
    else if (sigintrecibido.load())
    {
        registro->evento(0, "Servidor interrumpido. Partidas: " + partidas);
    }
    else
    {
        registro->evento(0, "Servidor finalizado. Partidas: " + partidas);
    }

    for (Cliente *cliente : clientes)
    {
        delete cliente;
    }

    delete matchmaker;
    delete sala;
    delete registro;
    delete cfg;
    delete args;
    pthread_mutex_destroy(&candadovigilante);

    return (error || fallovigilante.load()) ? 1 : 0;
}