#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include "ficha.hpp"

using namespace std;
// ############################# VARIABLES GLOBALES #############################
// --> Numero de fichas
const int nFichas =24;

// --> Configuraciones de la ventana y posiciones de jugadores
const float board_size = 800;
const float field_size = 80;
const float border_size = 50; 
    
// ############################# CLASES #############################
enum TipoMovimiento{
    INVALID,
    NORMAL,
    BEAT,
    MULTI_BEAT,
};

struct Movimiento{//para definir el movimiento y su tipo
    Movimiento() = default;
    Movimiento(sf::Vector2i inicio_, sf::Vector2i fin_, TipoMovimiento tipo_): inicio(inicio_), fin(fin_), tipo(tipo_){};
    sf::Vector2i inicio;
    sf::Vector2i fin;
    TipoMovimiento tipo;
};

class Tablero_Class{
    public:
        shared_ptr<Ficha> field[8][8]= {nullptr}; // tablero
        vector<weak_ptr<Ficha>> pawn_vector;
        vector<weak_ptr<Ficha>> player_pawns[2];
        bool beat_possible [2] = {false};

        Tablero_Class(){
            //Number of pawn and initialization for shapes and movement
            pawn_vector.reserve(nFichas);
            
            float new_x, new_y;

            OwningPlayer new_player;
            shared_ptr<Ficha> new_ptr;
            for (int i = 0; i < 8; ++i){
                for (int j = 0; j < 8; ++j){
                    if (i%2 == j%2){
                        if (j < 3 || j > 4){
                            //Create every position and board to every player
                            new_x = border_size + i * field_size + 5;
                            new_y = border_size + (7-j) * field_size + 5;
                            if (j < 3)
                                new_player = HUMAN;
                            else if (j > 4)
                                new_player = COMPUTER;
                            new_ptr = std::make_shared<Ficha>(i, j, new_x, new_y, new_player);
                            field[i][j] = new_ptr;
                            pawn_vector.push_back(weak_ptr<Ficha>(new_ptr));
                            //Separate for every player a pawns to rest next
                            getVector(new_player).push_back(std::weak_ptr<Ficha>(new_ptr));
                        }
                    }
                }
            }
        }

        Tablero_Class(const Tablero_Class& copied){
            //Simple copied od board, it uses when calculates possibilities
            for (int i = 0; i < 8; ++i){
                for (int j = 0; j < 8; ++j){
                    if (copied.field[i][j]){
                        shared_ptr<Ficha> new_ptr = shared_ptr<Ficha>(new Ficha(*copied.field[i][j]));
                        field[i][j] = new_ptr;
                        pawn_vector.push_back(weak_ptr<Ficha>(new_ptr));

                        getVector(new_ptr->owner).push_back(std::weak_ptr<Ficha>(new_ptr));
                    }
                }
            }
        }
        
        void print(){
            for (int y = 7; y > -1 ; --y){
                for (int x = 0; x < 8; ++x){
                    auto printed_pawn = getPawn(sf::Vector2i(x, y));
                    if (printed_pawn){
                        if (printed_pawn->owner == HUMAN)
                            std::cerr << 'O';
                        else
                            std::cerr << 'X';
                    }
                    else
                        std::cerr << ' ';
                }
                std::cerr << '\n';
            }
            std::cerr << '\n';
        }

        vector<weak_ptr<Ficha>>& getVector (OwningPlayer player){
            if (player == HUMAN)
                return player_pawns[0];
            else
                return player_pawns[1];
        }
        
        shared_ptr<Ficha> getPawn(const sf::Vector2i& coords){
            //Found the possibility to put Pawn depends on empty coords
            if (field[coords.x][coords.y] != nullptr)
                return field[coords.x][coords.y];
            else
                return nullptr;
        }

        TipoMovimiento checkMove(sf::Vector2i& inicio, sf::Vector2i& fin){
            //Every disable movement is writes here
            TipoMovimiento result = INVALID;
            if(fin.x >= 0 && fin.x <= 7 && fin.y >= 0 && fin.y <=7){
                if(std::shared_ptr<Ficha> pawn = getPawn(inicio)){
                    int direction = 1;
                    if (pawn->owner == COMPUTER)
                        direction = -1;
                    if (fin.y == inicio.y + direction){
                        if (fin.x == inicio.x + 1 || fin.x == inicio.x - 1){
                            if (!getPawn(fin)){
                                if(!getBeatPossible(pawn->owner)){
                                    result = NORMAL;
                                }
                            }
                        }
                    }
                    else if (fin.y == inicio.y + 2*direction){
                        if (fin.x == inicio.x + 2 || fin.x == inicio.x - 2){
                            if (!getPawn(fin)){
                                sf::Vector2i beaten_pawn(inicio.x + (fin.x - inicio.x)/2, inicio.y + direction);
                                if (getPawn(beaten_pawn)){
                                    if (getPawn(beaten_pawn)->owner == otherPlayer(pawn->owner)){
                                        result = BEAT;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return result;
        }

        vector<Movimiento>* getAvailibleMoves(OwningPlayer player, const shared_ptr<Ficha> pawn){
            vector<Movimiento>* move_vector = new vector<Movimiento>;
            sf::Vector2i inicio, fin;
            int direction = 1;
            if (pawn){
                if (player == COMPUTER)
                    direction = -1;
                inicio = pawn->coordinates;
                for (int k: {1,2}){
                    for (int l: {-1,1}){
                        fin = inicio + sf::Vector2i(l*k, k*direction);
                        // std::cerr << inicio.x << ' ' << inicio.y << ' ' << fin.x <<  ' ' << fin.y;
                        TipoMovimiento result = checkMove(inicio, fin);
                        if (result != INVALID){
                            Movimiento new_move = Movimiento(inicio, fin, result);
                            // std::cerr << " valid move";
                            move_vector->push_back(new_move);
                        }
                        // std::cerr << '\n';
                    }
                }
            }
            return move_vector;
        }
        
        vector<Movimiento>* getAvailibleMoves(OwningPlayer player){
            //Movements send to a vector to keep
            vector<Movimiento>* move_vector = new vector<Movimiento>;
            for (auto pawn_ptr: getVector(player)){
                if (auto pawn = pawn_ptr.lock()){
                    auto new_moves = getAvailibleMoves(player, pawn);
                    if(!new_moves->empty())
                        move_vector->insert(move_vector->end(), new_moves->begin(), new_moves->end());
                    delete new_moves;
                }
            }
            return move_vector;
        }

        bool& getBeatPossible(OwningPlayer player){
            if (player == COMPUTER)
                return beat_possible[1];
            return beat_possible[0];
        }
        
        void resolveBeating(OwningPlayer player){
            getBeatPossible(player) = false;
            std::vector<Movimiento>* move_vector = getAvailibleMoves(player);
            for (auto tested_move: *move_vector){
                if (tested_move.tipo == BEAT)
                    getBeatPossible(player) = true;
            }
        }

        int setPawn(const sf::Vector2i& coords, const std::shared_ptr<Ficha>& new_ptr){
            field[coords.x][coords.y] = new_ptr;
            return 0;
        }
        
        shared_ptr<Ficha> movePawn(sf::Vector2i inicio, sf::Vector2i fin, TipoMovimiento tipo){

            if (auto pawn = getPawn(inicio)){
                int direction = 1;
                if (pawn->owner == COMPUTER)
                    direction = -1;
                if(tipo == BEAT){
                    sf::Vector2i beaten_pawn(inicio.x + (fin.x - inicio.x)/2, inicio.y + direction);
                    getPawn(beaten_pawn).reset();
                    setPawn(beaten_pawn, nullptr);
                }
                setPawn(inicio, nullptr);
                setPawn(fin, pawn);
                pawn->coordinates = fin;
                resolveBeating(pawn->owner);

                return pawn;
            }
            return nullptr;
        }
        
        shared_ptr<Ficha> movePawn(const Movimiento& move){
            return movePawn(move.inicio, move.fin, move.tipo);
        }

        int getScore(OwningPlayer player){
            int score = 0;
            for (auto pawn_weak: getVector(player)){
                if (auto pawn = pawn_weak.lock()){
                    vector<Movimiento>* move_vector = new std::vector<Movimiento>;
                    int x = pawn->coordinates.x;
                    int y = pawn->coordinates.y;
                    score += 10;
                    if (player == HUMAN){
                        if (y == 2 || y == 3)
                            score += 1;
                        else if (y == 4 || y == 5)
                            score += 3;
                        else if (y == 6 || y == 7)
                            score += 5;
                    }
                    else{
                        if (y == 5 || y == 4)
                            score += 1;
                        else if (y == 3 || y == 2)
                            score += 3;
                        else if (y == 1 || y == 0)
                            score += 5;
                    }
                    if ((x == 0 || x == 7) && (y == 0 || y == 7))
                        score += 2;
                    else if ((x == 1 || x == 6) && (y == 1 || y == 6))
                        score += 1;
                    move_vector = getAvailibleMoves(player, pawn);
                    if (!move_vector->empty()){
                        for (auto tested_move: *move_vector){
                            if (tested_move.tipo == BEAT)
                                score += 30;
                        }
                    }
                    delete move_vector;
                }
            }
            return score;
        }

        OwningPlayer checkWin(OwningPlayer player){
            resolveBeating(player);
            OwningPlayer winner = NOBODY;
            vector<Movimiento>* availible_moves;

            availible_moves = getAvailibleMoves(player);
            if (availible_moves->empty()){
                winner = otherPlayer(player);
            }
            else {
                int pawn_count = 0;
                for (auto checked_pawn: getVector(player)){
                    if (!checked_pawn.expired())
                        ++pawn_count;
                }
                if(!pawn_count){
                    winner = otherPlayer(player);
                }
            }
            delete availible_moves;
            return winner;
        }
        
        //std::shared_ptr<Pawn> field[8][8]= {nullptr};
        //std::vector<std::weak_ptr<Pawn>> pawn_vector;
};