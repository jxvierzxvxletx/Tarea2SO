#ifndef MESA_HPP
#define MESA_HPP
#include "cfg.hpp"
#include "tablero.hpp"
#include "juez.hpp"
#include <pthread.h>
#include <array>
#include <queue>
using namespace std;
class Registro;

enum class Estado
{
    sinIniciar,
    enjuego,
    terminada
};

enum class Ganador
{
    Ninguno,
    puma,
    perros
};



class Mesa
{
private:

    Tablero tablero;
    Juez juez;
    Estado e_mesa = Estado::sinIniciar;
    pthread_mutex_t candado;
    pthread_cond_t aviso;
    queue<array<int, 5>> solicitudes;
    Ganador ganador = Ganador:: Ninguno;
    Registro * registro = nullptr;
    int idmesa = 0;
    int idpartida = 0;
    

;

public:

void configurarRegistro(Registro * registro, int mesa, int partida);
bool iniciar(Cfg &datos);

bool f_quierejugar(int ficha, int inicio, int final,bool es_captura, int posicion_perro);

bool sacarsoli(array<int, 5> &solicitud);
void mesacerrada();

bool procesarsoli(const array<int, 5> &pedido);

void juezfuncionando();

Mesa();
~Mesa();

bool copiartablero(Tablero &copia);



  
};

void *hiloJuez(void *arg);

bool jugarpartida(Mesa &mesa, int retardomin, int retardomax,unsigned int semilla);

#endif