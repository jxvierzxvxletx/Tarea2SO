#include "tablero.hpp"

Tablero::Tablero()
{
    for (int i = 0; i < 28; i++)
    {
        posiciones[i] = 0;
    }

    for (int i = 0; i < 28; i ++)
    {
        for (int j = 0; j < 28; j ++)
        {
            conexiones[i][j] = false;
        }
        

    }

    for (int t = 0; t < 25; t ++)
    {
        if (t % 5 != 4)
        {
            conexiones[t][t + 1] = true;
            conexiones[t + 1][t] = true;
        }
    }

    for (int t = 0; t < 20; t ++)
    {
    
        conexiones[t][t + 5] = true;
        conexiones[t + 5][t] = true;
        
    }
    //25
    conexiones[25][26] = true;
    conexiones[26][25] = true;
    conexiones[25][2] = true;
    conexiones[2][25] = true;
    conexiones[25][27] = true;
    conexiones[27][25] = true;

    //27
    conexiones[27][3] = true;
    conexiones[3][27] = true;
    conexiones[27][4] = true;
    conexiones[4][27] = true;
    conexiones[27][26] = true;
    conexiones[26][27] = true;

    //26
    conexiones[26][1] = true;
    conexiones[1][26] = true;
    conexiones[26][0] = true;
    conexiones[0][26] = true;
    
    

}


bool Tablero::seconectan(int inicio, int final)
{
    if (inicio < 0 || inicio >= 28 || final < 0 || final >= 28)
    {
        return false;
    }

    return conexiones[inicio][final];
}

int Tablero:: consultarposicion (int posicion ) //saber que hay dentro dde esa posicion
{
    if(posicion < 0  || posicion >= 28)
    {
        return -1;
    }
    else
    {
        return posiciones[posicion];
    }

}

bool Tablero::colocarficha(int posicion, int ficha) // iniciar tablero
{
    

    if( posicion < 0 || posicion >=28 || ficha < 1 || ficha > 13  )
    {
        return false;
    }
    if ( posiciones[posicion] != 0 )
    {
        return false;
    }

    for(int i = 0; i < 28; i ++)
    {
        if(posiciones[i] == ficha )
        {
            return false;
        }
    }

    
        posiciones[posicion] = ficha;
        return true;
    
}

bool Tablero::moverlaficha(int inicio, int final)
{
    if( inicio < 0 || inicio >=28 || final < 0 || final >= 28 )
    {
        return false;
    }
    else if (posiciones[inicio] == 0 || posiciones[final] != 0)
    {
        return false;
    }

    
    posiciones[final] = posiciones[inicio];
    posiciones[inicio] = 0;
    
    return true;
}

bool Tablero::fichamuerta(int posicion)
{
    if( posicion < 0 || posicion >=28 || posiciones[posicion] == 0 )
    {
        return false;
    }

    posiciones[posicion] = 0;
    return true;

}

bool Tablero::feet(Cfg &datos)
{
    int puma = datos.getPosicionPuma();

    if(colocarficha(puma,1) == false)
    {
        return false;
    }

    vector<int> perros = datos.getPosicionesPerros();

    if (perros.size() != 12)
    {
        fichamuerta(puma); //se borra pq ya se habia colocado
        return false;
    }

    for(int i = 0; i < 12; i ++)
    {
        int posicion_p = perros[i];
       

        if(colocarficha(posicion_p, i + 2) == false)
        {
           for (int j = 0;  j < i; j ++)
            {
                fichamuerta(perros[j]); //borrar los ya colocados
            }
            fichamuerta(puma);
            return false;
            
        }
    }

    return true;

}