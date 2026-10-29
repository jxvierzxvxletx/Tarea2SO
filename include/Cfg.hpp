// cfg.hpp - [A] Configuracion de entrada (informe 3.1)
#ifndef CFG_HPP
#define CFG_HPP
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Formato de config.txt (una palabra clave por linea):
//   retardomin 50
//   retardomax 300
//   semilla 42
//   puma 12
//   perros 0 1 2 3 4 5 6 7 8 9 10 11
class Cfg
{
    private:
        // Datos fijos que da el informe (2.1 y 2.2)
        int filas;
        int columnas;
        int casillastrono;        // SUPUESTO: confirmar con el diagrama del informe
        int cantidadperros;
        int capturasvictoria;
        // Datos que puede cambiar el config.txt (3.1)
        int retardomin;           // ms
        int retardomax;           // ms
        unsigned int semilla;
        int posicionpuma;
        vector <int> posicionesperros;
    public:
        Cfg();
        bool cargarConfig(string ruta);
        int getFilas()
        {
            return filas;
        }
        int getColumnas()
        {
            return columnas;
        }
        int getCasillasTrono()
        {
            return casillastrono;
        }
        int getTotalCasillas()
        {
            return filas * columnas + casillastrono;
        }
        int getCantidadPerros()
        {
            return cantidadperros;
        }
        int getCapturasVictoria()
        {
            return capturasvictoria;
        }
        int getRetardoMin()
        {
            return retardomin;
        }
        int getRetardoMax()
        {
            return retardomax;
        }
        unsigned int getSemilla()
        {
            return semilla;
        }
        int getPosicionPuma()
        {
            return posicionpuma;
        }
        vector <int> getPosicionesPerros()
        {
            return posicionesperros;
        }
};

#endif