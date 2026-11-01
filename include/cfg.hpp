 #ifndef CFG_HPP
#define CFG_HPP
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Formato de config.txtd
//   retardomin 50
//   retardomax 300
//   semilla 42
//   puma 25
//   perros 0 1 2 3 4 5 6 7 8 9 10 11
// Si no hay config.txt (o no trae "perros"), los 12 perros se reparten al azar
// entre las posiciones 0 a 24. Si no trae "puma", el puma empieza en la 25.
class Cfg
{
    private:
        int filas;
        int columnas;
        int casillastrono;        // 3: posiciones 25 (T), 26 (SI) y 27 (SD)
        int cantidadperros;
        int capturasvictoria;
        int retardomin;           // ms
        int retardomax;           // ms
        unsigned int semilla;
        int posicionpuma;
        vector <int> posicionesperros;
        void repartirPerrosAlAzar();
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