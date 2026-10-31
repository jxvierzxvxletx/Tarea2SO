#include "cfg.hpp"
#include <fstream>
#include <stdlib.h>

// Valores por defecto (cuando no hay config.txt):
// retardos de 50 a 300 ms (informe), puma en la 25 y perros al azar entre 0 y 24.
Cfg::Cfg()
{
    filas = 5;
    columnas = 5;
    casillastrono = 3;
    cantidadperros = 12;
    capturasvictoria = 6;
    retardomin = 50;
    retardomax = 300;
    semilla = 1;
    posicionpuma = 25;
    repartirPerrosAlAzar();
}

// Elige 12 casillas distintas de la cuadricula (0 a 24) para los perros.
// Usa la semilla, asi que con la misma semilla sale siempre la misma disposicion.
void Cfg::repartirPerrosAlAzar()
{
    unsigned int estado = semilla;
    vector <int> libres;
    for (int i = 0; i < filas * columnas; i++)
    {
        if (i != posicionpuma)
        {
            libres.push_back(i);
        }
    }
    posicionesperros.clear();
    for (int i = 0; i < cantidadperros; i++)
    {
        int tam = libres.size();
        int indice = rand_r(&estado) % tam;
        posicionesperros.push_back(libres[indice]);
        libres.erase(libres.begin() + indice);
    }
}

bool Cfg::cargarConfig(string ruta)
{
    ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        cout << "Error: no se pudo abrir el archivo de configuracion " << ruta << endl;
        return false;
    }

    // Se lee "palabra valor(es)" hasta que se acabe el archivo, igual que con cin
    bool leyoperros = false;
    string clave;
    while (archivo >> clave)
    {
        if (clave == "retardomin")
        {
            archivo >> retardomin;
        }
        else if (clave == "retardomax")
        {
            archivo >> retardomax;
        }
        else if (clave == "semilla")
        {
            archivo >> semilla;
        }
        else if (clave == "puma")
        {
            archivo >> posicionpuma;
        }
        else if (clave == "perros")
        {
            leyoperros = true;
            posicionesperros.clear();
            for (int i = 0; i < cantidadperros; i++)
            {
                int pos;
                archivo >> pos;
                posicionesperros.push_back(pos);
            }
        }
        else
        {
            cout << "Error: palabra desconocida en la configuracion: " << clave << endl;
            return false;
        }
        if (archivo.fail())
        {
            cout << "Error: valor invalido o faltante en: " << clave << endl;
            return false;
        }
    }

    // Si el archivo no dice donde van los perros, se reparten al azar con la semilla ya leida
    if (leyoperros == false)
    {
        repartirPerrosAlAzar();
    }

    // Retardos coherentes
    if (retardomin < 0 || retardomax < retardomin)
    {
        cout << "Error: retardos invalidos (debe cumplirse 0 <= retardomin <= retardomax)" << endl;
        return false;
    }

    // Disposicion inicial: puma + perros, todos en casillas que existan y sin repetir
    vector <int> todas = posicionesperros;
    todas.push_back(posicionpuma);
    int total = todas.size();
    for (int i = 0; i < total; i++)
    {
        if (todas[i] < 0 || todas[i] >= getTotalCasillas())
        {
            cout << "Error: la casilla " << todas[i] << " no existe (validas: 0 a " << getTotalCasillas() - 1 << ")" << endl;
            return false;
        }
        for (int j = 0; j < i; j++)
        {
            if (todas[i] == todas[j])
            {
                cout << "Error: la casilla " << todas[i] << " esta repetida en la disposicion inicial" << endl;
                return false;
            }
        }
    }
    return true;
}