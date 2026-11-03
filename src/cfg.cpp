/#include "../include/cfg.hpp"
#include <fstream>
#include <stdlib.h>

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
       cout << "Error, no se pudo abrir el archivo de configuracion " << ruta << endl;
        return false;
    }

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

                if (!(archivo >> pos))
                {
                    cout << "Error, valor invalido o faltante en: perros" << endl;
                    return false;
                }

                posicionesperros.push_back(pos);
            }
        }
        else
        {
            cout << "Error, palabra desconocida en la configuracion: " << clave << endl;
            return false;
        }
        if (archivo.fail())
        {
            cout << "Error, valor invalido o faltante en: " << clave << endl;
            return false;
        }
    }

    if (leyoperros == false)
    {
        repartirPerrosAlAzar();
    }

    if (retardomin < 0 || retardomax < retardomin)
    {
        cout << "Error, retardos invalidos (debe cumplirse 0 <= retardomin <= retardomax)" << endl;
        return false;
    }

    vector <int> todas = posicionesperros;
    todas.push_back(posicionpuma);
    int total = todas.size();
    for (int i = 0; i < total; i++)
    {
        if (todas[i] < 0 || todas[i] >= getTotalCasillas())
        {
            cout << "Error, la casilla " << todas[i] << " no existe (validas: 0 a " << getTotalCasillas() - 1 << ")" << endl;
            return false;
        }
        for (int j = 0; j < i; j++)
        {
            if (todas[i] == todas[j])
            {
                cout << "Error, la casilla " << todas[i] << " esta repetida en la disposicion inicial" << endl;
                return false;
            }
        }
    }
    return true;
}