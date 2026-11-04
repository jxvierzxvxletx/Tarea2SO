#include "tablero.hpp"
#include"juez.hpp"
#include "mesa.hpp"
#include "../include/logs.hpp"
#include <vector>
#include <string>
#include <pthread.h>


void Mesa::configurarRegistro(Registro * r, int mesa, int partida)
{
    registro = r;
    idmesa = mesa;
    idpartida = partida;
}

bool Mesa::iniciar(Cfg &datos)
{
    if ( e_mesa != Estado::sinIniciar || tablero.feet(datos) == false )
    {
        return false;
    }

    e_mesa = Estado::enjuego;
    if (juez.loosepuma(tablero))
    {
        ganador = Ganador::perros;
        e_mesa = Estado::terminada;
    }
    if (registro != nullptr)
    {
        vector<int> casillas;
        for (int i = 0; i < 28; i++)
        {
            casillas.push_back(tablero.consultarposicion(i));
        }
        registro->eventoTablero(idmesa, "Partida " + to_string(idpartida) + ": tablero inicial", casillas);
        if (ganador == Ganador::perros)
        {
            registro->evento(idmesa, "Partida " + to_string(idpartida) + ": ganan los perros por bloqueo inicial");
        }
    }
    return true;


}

bool Mesa::f_quierejugar(int ficha, int inicio, int final, bool es_captura, int posicion_perro)
{
    pthread_mutex_lock(&candado);

    if (e_mesa != Estado::enjuego)
    {
        pthread_mutex_unlock(&candado);
        return false;
    }

    solicitudes.push({ficha, inicio, final, es_captura, posicion_perro});
    pthread_cond_signal(&aviso);
    pthread_mutex_unlock(&candado);

    return true;
};

bool Mesa::sacarsoli(array<int, 5> &solicitud)
{
    pthread_mutex_lock(&candado);

    while(e_mesa == Estado::enjuego && solicitudes.empty() && abandonoPendiente == 0)
    {
         pthread_cond_wait(&aviso, &candado);
         
    }

    if ( e_mesa != Estado::enjuego)
    {
        pthread_mutex_unlock(&candado);
        return false;

    }

    if (abandonoPendiente != 0)
    {
        solicitud = {-1, abandonoPendiente, 0, 0, 0};
        abandonoPendiente = 0;
    }
    else
    {
        solicitud = solicitudes.front();
        solicitudes.pop();
    }
    pthread_mutex_unlock(&candado);
    return true;


}

void Mesa::mesacerrada()


{
    pthread_mutex_lock(&candado);

    e_mesa = Estado::terminada;
    pthread_cond_signal (&aviso);


    pthread_mutex_unlock(&candado);

}

bool Mesa::notificarAbandono(const string &rol)
{
    if (rol != "puma" && rol != "perros")
    {
        return false;
    }

    pthread_mutex_lock(&candado);
    if (e_mesa != Estado::enjuego || abandonoPendiente != 0)
    {
        pthread_mutex_unlock(&candado);
        return false;
    }

    abandonoPendiente = rol == "puma" ? 1 : 2;
    pthread_cond_signal(&aviso);
    pthread_mutex_unlock(&candado);
    return true;
}

bool Mesa::procesarsoli(const array<int, 5> &pedido)
{
        pthread_mutex_lock(&candado);

        if (e_mesa != Estado::enjuego)
        {
            pthread_mutex_unlock(&candado);
            return false;
        }

        if (pedido[0] == -1)
        {
            bool abandonoPuma = pedido[1] == 1;
            ganador = abandonoPuma ? Ganador::perros : Ganador::puma;
            e_mesa = Estado::terminada;
            pthread_cond_broadcast(&aviso);
            pthread_mutex_unlock(&candado);

            if (registro != nullptr)
            {
                string mensaje = abandonoPuma
                    ? "Partida " + to_string(idpartida) + ": ganan los perros por abandono del Puma"
                    : "Partida " + to_string(idpartida) + ": gana el Puma por abandono de los perros";
                registro->evento(idmesa, mensaje);
            }
            return true;
        }

        int  ficha          = pedido[0];
        int inicio         = pedido[1];
        int final          = pedido[2];
        bool es_captura     = pedido[3] == 1;
        int posicion_perro = pedido[4];
        bool resultado = false;

         if (es_captura)
         {
            

            if( ficha == 1)
            {
                resultado = juez.capturar(tablero, inicio, posicion_perro,final);

            }
            
         }
         else 
        {
            resultado = juez.hacermov(tablero, ficha, inicio, final);

        }



        if(resultado == true)
        {
            if(juez.winpuma() == true)
            {
                ganador = Ganador::puma;
            }
            else if (juez.loosepuma(tablero)== true)
            {
                ganador = Ganador::perros;
            }
            
            
        }

        if(ganador != Ganador::Ninguno)
        {
            e_mesa = Estado::terminada;
            pthread_cond_signal(&aviso);

        }

        vector<int> casillas;
        Ganador resultadoGanador = ganador;
        if (resultado && registro != nullptr)
        {
            for (int i = 0; i < 28; i++)
            {
                casillas.push_back(tablero.consultarposicion(i));
            }
        }
        pthread_mutex_unlock(&candado);

        if (resultado && registro != nullptr)
        {
            string nombreFicha = ficha == 1 ? "Puma" : "Perro " + to_string(ficha - 1);
            string mensaje = "Partida " + to_string(idpartida) + ": ";
            mensaje += es_captura ? "[CAPTURA] " : "[MOVIMIENTO] ";
            mensaje += nombreFicha + " | posicion " + to_string(inicio) + " -> " + to_string(final);
            if (es_captura)
            {
                mensaje += " | perro capturado en " + to_string(posicion_perro);
            }
            registro->eventoTablero(idmesa, mensaje, casillas);
            if (resultadoGanador == Ganador::puma)
            {
                registro->evento(idmesa, "Partida " + to_string(idpartida) + ": gana el Puma por seis capturas");
            }
            else if (resultadoGanador == Ganador::perros)
            {
                registro->evento(idmesa, "Partida " + to_string(idpartida) + ": ganan los perros por bloqueo del Puma");
            }
        }
        return resultado;




}

void Mesa::juezfuncionando()
{
    array<int, 5> pedido{};

    while (sacarsoli(pedido) == true)
    {
        procesarsoli(pedido);
    }
}


Mesa::Mesa()
{
    pthread_mutex_init(&candado, NULL);
    pthread_cond_init(&aviso, NULL);
}

Mesa::~Mesa()
{
    pthread_cond_destroy(&aviso);
    pthread_mutex_destroy(&candado);

}



void * hiloJuez(void * arg)
{
    Mesa * mesa = static_cast< Mesa *>(arg);
    mesa->juezfuncionando();
    return NULL;
}

bool Mesa::copiartablero(Tablero &copia)
{
    pthread_mutex_lock(&candado);

    if (e_mesa != Estado::enjuego)
    {
        pthread_mutex_unlock(&candado);
        return false;
    }

    copia = tablero;

    pthread_mutex_unlock(&candado);
    return true;
}






