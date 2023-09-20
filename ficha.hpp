#ifndef CHECKERSV_2_PAWN_HPP
#define CHECKERSV_2_PAWN_HPP

#include <SFML/Graphics.hpp>

// --> Guaramos los tipos de jugadores que tenemos
enum TipoJugador{
    winnerPlayer,
    personPlayer,
    IAPlayer,
};

TipoJugador otherPlayer(TipoJugador current_player){
    if (current_player == personPlayer)
        return IAPlayer;
    return personPlayer;
}

class Ficha{
public:
    TipoJugador owner;
    float x,y;
    sf::Vector2i coordinates;

    //Positions
    Ficha(int coord_x, int coord_y, float x_, float y_, TipoJugador owner_){
        owner = owner_;
        x = x_;
        y = y_;
        coordinates.x = coord_x;
        coordinates.y = coord_y;
    }

};

#endif
