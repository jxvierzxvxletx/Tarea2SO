// cfg.cpp - [A] Configuracion de entrada (informe 3.1)
#include "cfg.hpp"
#include <fstream>

// Valores por defecto. El informe dice que el archivo es OPCIONAL y que los
// retardos van entre 50 y 300 ms. La posicion inicial por defecto es un SUPUESTO
// (el informe la deja al config.txt): puma al centro, perros en las casillas 0 a 11.
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
    posicionpuma = 12;
    for (int i = 0; i < cantidadperros; i++)
    {
        posicionesperros.push_back(i);
    }
}

bool Cfg::cargarConfig(string ruta)
{
    ifstream archivo(ruta);
    if (!archivo.is_open())
    {
        cout << "Error no se pudo abrir el archivo de configuracion " << ruta << endl;
        return false;
    }

    // Se lee "palabra valor(es)" hasta que se acabe el archivo, igual que con cin
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
            cout << "Error palabra desconocida en la configuracion: " << clave << endl;
            return false;
        }
        if (archivo.fail())
        {
            cout << "Error valor invalido o faltante en: " << clave << endl;
            return false;
        }
    }

    // Retardos coherentes
    if (retardomin < 0 || retardomax < retardomin)
    {
        cout << "Error retardos invalidos (debe cumplirse 0 <= retardomin <= retardomax)" << endl;
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
            cout << "Error la casilla " << todas[i] << " no existe (validas: 0 a " << getTotalCasillas() - 1 << ")" << endl;
            return false;
        }
        for (int j = 0; j < i; j++)
        {
            if (todas[i] == todas[j])
            {
                cout << "Error la casilla " << todas[i] << " esta repetida en la disposicion inicial" << endl;
                return false;
            }
        }
    }
    return true;
}