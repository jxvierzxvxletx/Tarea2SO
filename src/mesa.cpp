#include "tablero.hpp"
#include"juez.hpp"
#include "mesa.hpp"
#include <pthread.h>


bool Mesa::iniciar(Cfg &datos)
{
    if ( e_mesa != Estado::sinIniciar || tablero.feet(datos) == false )
    {
        return false;
    }

    e_mesa = Estado ::enjuego;
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

    while(e_mesa == Estado::enjuego && solicitudes.empty())
    {
         pthread_cond_wait(&aviso, &candado);
         
    }

    if ( e_mesa != Estado::enjuego)
    {
        pthread_mutex_unlock(&candado);
        return false;

    }

    solicitud = solicitudes.front();
    solicitudes.pop();
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

bool Mesa::procesarsoli(const array<int, 5> &pedido)
{
        pthread_mutex_lock(&candado);

        if (e_mesa != Estado::enjuego)
        {
            pthread_mutex_unlock(&candado);
            return false;
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

        pthread_mutex_unlock(&candado);
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








