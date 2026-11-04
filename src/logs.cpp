#include "../include/logs.hpp"
#include <cstdlib>
#include <unistd.h>

namespace
{
    bool coloresActivos()
    {
        return isatty(STDOUT_FILENO) == 1 && getenv("NO_COLOR") == nullptr;
    }

    string numeroDosDigitos(int numero)
    {
        if (numero < 10)
        {
            return "0" + to_string(numero);
        }
        return to_string(numero);
    }

    void agregarLinea(string &salida, const string &contenido, bool centrar)
    {
        const size_t ancho = 61;
        size_t espacios = contenido.size() < ancho ? ancho - contenido.size() : 0;
        size_t izquierda = centrar ? espacios / 2 : 0;
        size_t derecha = espacios - izquierda;
        salida += "    | " + string(izquierda, ' ') + contenido + string(derecha, ' ') + " |\n";
    }

    string pintarMensaje(const string &mensaje)
    {
        if (!coloresActivos())
        {
            return mensaje;
        }
        if (mensaje.find("gana el Puma") != string::npos ||
            mensaje.find("ganan los perros") != string::npos)
        {
            return "\033[1;32m" + mensaje + "\033[0m";
        }
        if (mensaje.find("Error") != string::npos ||
            mensaje.find("error") != string::npos)
        {
            return "\033[1;31m" + mensaje + "\033[0m";
        }
        if (mensaje.find("[CAPTURA]") != string::npos)
        {
            return "\033[1;33m" + mensaje + "\033[0m";
        }
        return mensaje;
    }
}

Registro::Registro(Cfg *configuracion)
{
    cfg = configuracion;
    pthread_mutex_init(&candado, nullptr);
}

Registro::~Registro()
{
    pthread_mutex_destroy(&candado);
}

string Registro::aTexto(int numero)
{
    return to_string(numero);
}

string Registro::simbolo(int valor)
{
    if (valor == 0)
    {
        return " .  ";
    }
    if (valor == 1)
    {
        return "PUMA";
    }
    if (valor >= 2 && valor <= cfg->getCantidadPerros() + 1)
    {
        return "D" + numeroDosDigitos(valor - 1) + " ";
    }
    return "????";
}

void Registro::evento(int mesa, string mensaje)
{
    string etiqueta = mesa > 0 ? "[Mesa " + aTexto(mesa) + "] " : "[Servidor] ";

    pthread_mutex_lock(&candado);

    if (mesa > 0 && mensaje.find("asignada:") != string::npos)
    {
        cout << "\n" << string(69, '=') << "\n";
    }

    if (coloresActivos())
    {
        cout << (mesa > 0 ? "\033[1;36m" : "\033[1;35m") << etiqueta << "\033[0m";
    }
    else
    {
        cout << etiqueta;
    }
    cout << pintarMensaje(mensaje) << '\n';

    if (mensaje == "Partida finalizada")
    {
        cout << string(69, '=') << "\n\n";
    }

    pthread_mutex_unlock(&candado);
}

string Registro::textoTablero(int mesa, const vector<int> &casillas)
{
    if (casillas.size() != static_cast<size_t>(cfg->getTotalCasillas()))
    {
        return "[Mesa " + aTexto(mesa) + "] Error: tablero de tamano incorrecto";
    }

    auto celda = [&](int posicion)
    {
        return "[" + numeroDosDigitos(posicion) + ":" + simbolo(casillas[posicion]) + "]";
    };

    const string borde = "    +" + string(63, '-') + "+\n";
    string resultado = "[Mesa " + aTexto(mesa) + "] Tablero:\n";
    resultado += borde;
    agregarLinea(resultado, "TABLERO KOMIKAN - MESA " + aTexto(mesa), true);
    agregarLinea(resultado, "TRONO", true);
    agregarLinea(resultado, celda(25), true);
    agregarLinea(resultado, celda(26) + "   " + celda(27), true);
    agregarLinea(resultado, "CUADRICULA PRINCIPAL", true);

    for (int fila = 0; fila < cfg->getFilas(); fila++)
    {
        string contenido;
        for (int columna = 0; columna < cfg->getColumnas(); columna++)
        {
            if (columna > 0)
            {
                contenido += " ";
            }
            contenido += celda(fila * cfg->getColumnas() + columna);
        }
        agregarLinea(resultado, contenido, true);
    }

    agregarLinea(resultado, "PUMA = Puma    D01-D12 = Perros    . = libre", true);
    resultado += borde;

    if (coloresActivos())
    {
        for (int posicion = 0; posicion < cfg->getTotalCasillas(); posicion++)
        {
            string casilla = celda(posicion);
            string color = casillas[posicion] == 1 ? "\033[1;33m" :
                           casillas[posicion] >= 2 ? "\033[1;36m" : "\033[2m";
            string pintada = "[" + numeroDosDigitos(posicion) + ":" +
                             color + simbolo(casillas[posicion]) + "\033[0m]";
            size_t indice = resultado.find(casilla);
            if (indice != string::npos)
            {
                resultado.replace(indice, casilla.size(), pintada);
            }
        }
    }

    return resultado;
}

void Registro::tablero(int mesa, vector<int> casillas)
{
    string contenido = textoTablero(mesa, casillas);
    pthread_mutex_lock(&candado);
    cout << contenido;
    pthread_mutex_unlock(&candado);
}

void Registro::eventoTablero(int mesa, string mensaje, const vector<int> &casillas)
{
    string contenido = textoTablero(mesa, casillas);
    string etiqueta = "[Mesa " + aTexto(mesa) + "] ";

    pthread_mutex_lock(&candado);
    if (coloresActivos())
    {
        cout << "\033[1;36m" << etiqueta << "\033[0m";
    }
    else
    {
        cout << etiqueta;
    }
    cout << pintarMensaje(mensaje) << '\n';
    cout << contenido;
    pthread_mutex_unlock(&candado);
}