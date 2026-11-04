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

string Registro::textoTablero(int mesa, const vector<int> & casillas)
{
    if (casillas.size() != static_cast<size_t>(cfg->getTotalCasillas()))
    {
        return "[Mesa " + aTexto(mesa) + "] Error: tablero de tamaño incorrecto";
    }
    string vacio = "     ";
    string texto = "[Mesa " + aTexto(mesa) + "] Tablero:\n";
    texto += vacio + vacio + "[" + simbolo(casillas[25]) + "]\n";
    texto += vacio + "[" + simbolo(casillas[26]) + "]" + vacio + "[" + simbolo(casillas[27]) + "]\n";
    int filas = cfg->getFilas();
    int columnas = cfg->getColumnas();
    for (int f = 0; f < filas; f++)
    {
        for (int c = 0; c < columnas; c++)
        {
            texto += "[" + simbolo(casillas[f * columnas + c]) + "]";
        }
        if (f < filas - 1)
        {
            texto += "\n";
        }
    }
    return texto;
}

void Registro::tablero(int mesa, vector<int> casillas)
{
    string texto = textoTablero(mesa, casillas);
    pthread_mutex_lock(&candado);
    cout << texto << endl;
    pthread_mutex_unlock(&candado);
}

void Registro::eventoTablero(int mesa, string mensaje, const vector<int> & casillas)
{
    string texto = textoTablero(mesa, casillas);
    pthread_mutex_lock(&candado);
    cout << "[Mesa " << aTexto(mesa) << "] " << mensaje << endl;
    cout << texto << endl;
    pthread_mutex_unlock(&candado);
}