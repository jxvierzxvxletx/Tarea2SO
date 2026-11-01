#include "../include/logs.hpp"

Registro::Registro(Cfg * cfg)
{
    this -> cfg = cfg;
    pthread_mutex_init(&candado, NULL);
}

Registro::~Registro()
{
    pthread_mutex_destroy(&candado);
}

string Registro::aTexto(int n)
{
    if (n == 0)
    {
        return "0";
    }
    bool negativo = false;
    if (n < 0)
    {
        negativo = true;
        n = -n;
    }
    string resultado = "";
    while (n > 0)
    {
        resultado = string(1, '0' + n % 10) + resultado;
        n = n / 10;
    }
    if (negativo)
    {
        resultado = "-" + resultado;
    }
    return resultado;
}

// Lo que se dibuja dentro de cada casilla (siempre de 3 letras):
// " . " libre, " P " puma, "D1 " a "D12" los perros (ids 2 a 13)
string Registro::simbolo(int valor)
{
    if (valor == 0)
    {
        return " Libre ";
    }
    if (valor == 1)
    {
        return " Puma ";
    }
    if (valor < 0 || valor > cfg -> getCantidadPerros() + 1)
    {
        return " ? ";
    }
    string texto = "Perro " + aTexto(valor - 1);
    while (texto.size() < 3)
    {
        texto = texto + " ";
    }
    return texto;
}

void Registro::evento(int mesa, string texto)
{
    string etiqueta = "[Servidor] ";
    if (mesa > 0)
    {
        etiqueta = "[Mesa " + aTexto(mesa) + "] ";
    }
    pthread_mutex_lock(&candado);
    cout << etiqueta << texto << endl;
    pthread_mutex_unlock(&candado);
}

// Imprime el tablero completo. casillas[i] dice que hay en la posicion i:
// 0 libre, 1 puma, 2 a 13 perros. Posiciones 0 a 24 = cuadricula, 25 = T, 26 = SI, 27 = SD.
void Registro::tablero(int mesa, vector <int> casillas)
{
    int total = casillas.size();
    if (total != cfg -> getTotalCasillas())
    {
        evento(mesa, "Error al imprimir, el tablero debe tener " + aTexto(cfg -> getTotalCasillas()) + " casillas y trae " + aTexto(total));
        return;
    }
    string vacio = "     ";
    string texto = "[Mesa " + aTexto(mesa) + "] Tablero:\n";
    texto = texto + vacio + vacio + "[" + simbolo(casillas[25]) + "]\n";
    texto = texto + vacio + "[" + simbolo(casillas[26]) + "]" + vacio + "[" + simbolo(casillas[27]) + "]\n";
    // Cuadricula: una linea por fila
    int filas = cfg -> getFilas();
    int columnas = cfg -> getColumnas();
    for (int f = 0; f < filas; f++)
    {
        for (int c = 0; c < columnas; c++)
        {
            texto = texto + "[" + simbolo(casillas[f * columnas + c]) + "]";
        }
        if (f < filas - 1)
        {
            texto = texto + "\n";
        }
    }
    pthread_mutex_lock(&candado);
    cout << texto << endl;
    pthread_mutex_unlock(&candado);
}