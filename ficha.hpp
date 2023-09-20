#ifndef CHECKERSV_2_PAWN_HPP
#define CHECKERSV_2_PAWN_HPP

#include <SFML/Graphics.hpp>

//Type of declaration of Players
enum OwningPlayer{
    NOBODY,
    HUMAN,
    COMPUTER,
};

//Sequence of play
OwningPlayer otherPlayer(OwningPlayer current_player){
    if (current_player == HUMAN)
        return COMPUTER;
    return HUMAN;
}

enum Nivel{
    normal,
    king,
};

class Ficha{
public:
    OwningPlayer owner;
    Nivel level = normal;
    float x,y;
    sf::Vector2i coordinates;

    //Positions
    Ficha(int coord_x, int coord_y, float x_, float y_, OwningPlayer owner_){
        owner = owner_;
        x = x_;
        y = y_;
        coordinates.x = coord_x;
        coordinates.y = coord_y;
    }

};

#endif
