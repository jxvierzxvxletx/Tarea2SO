// cliente.hpp - [A] Un jugador (cliente) que llega al servidor (informe 3.2)
#ifndef CLIENTE_HPP
#define CLIENTE_HPP
#include <iostream>
#include <string>
#include <pthread.h>
using namespace std;

class Cliente
{
    private:
        int id;
        string rol;                 // "puma", "perros" o "sinasignar"
        bool terminado;             // true cuando su partida ya termino
        pthread_mutex_t candado;    // protege a terminado
        pthread_cond_t aviso;       // despierta al cliente cuando su partida termina
    public:
        Cliente(int id);
        ~Cliente();
        void setRol(string r)
        {
            rol = r;
        }
        int getId()
        {
            return id;
        }
        string getRol()
        {
            return rol;
        }
        bool getTerminado();
        void esperarFin();          // el cliente se duerme hasta que su partida termine
        void avisarFin();           // la mesa llama esto cuando la partida termina
};

#endif