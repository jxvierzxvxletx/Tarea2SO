#ifndef LOGS_HPP
#define LOGS_HPP
#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include "cfg.hpp"
using namespace std;

class Registro
{
    private:
        Cfg * cfg;
        pthread_mutex_t candado;    // evitar que dos mesas se mezclen en lo que imprimen
        string aTexto(int n);
        string simbolo(int valor);
        string textoTablero(int mesa, const vector<int> & casillas);
    public:
        Registro(Cfg * cfg);
        ~Registro();
        void evento(int mesa, string texto);
        void tablero(int mesa, vector <int> casillas);
        void eventoTablero(int mesa, string texto, const vector<int> & casillas);
};

#endif