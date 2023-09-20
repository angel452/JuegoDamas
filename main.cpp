#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include "juego_damas.hpp"

using namespace std;

// ################################# MAIN #################################
int main(int argc, char const *argv[])
{
    Juego_damas juego;
    juego.inicio();
    juego.jugar();
    return 0;
}
