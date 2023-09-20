#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include "ficha.hpp"
#include "tablero.hpp"

// ################################# VARIABLES GLOBALES #################################
// --> Logica Poda Alfa-Beta
const int INF_Positivo = 10000;
const int INF_Negativo = -10000;

// ################################# CLASES #################################

void delay(int miliseconds){
    sf::Clock clock;
    clock.restart();
    while(1){
        if(clock.getElapsedTime().asMilliseconds() > miliseconds)
            break;
    }
}

class Juego_damas{
    public:
        // --> Objetos SFML - PARTE GRAFICA
        sf::Texture textures[5]; 
        sf::Sprite sprites[5];
        sf::Image icon;
        sf::RenderWindow window;

        TipoJugador players[2] = {personPlayer, IAPlayer};
        Tablero_Class game_board;
        //Who starts to play?
        TipoJugador active_player = personPlayer;

        Juego_damas(){
            icon.loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/FichaBlanca.png");

            // --> Texturas de las fichas
            textures[0].loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/tablero2.png");
            textures[1].loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/FichaNegra.png");
            textures[2].loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/FichaBlanca.png");
            // textures[3].loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/FichaNegra.png"); // REINA
            // textures[4].loadFromFile("/home/angel/Documentos/UCSP/Semestre7/InteligenciaArtificial/Damas/Checkersv-master/graphics/FichaBlanca.png"); // REINA

            // --> Cargamos el tablero
            sprites[0].setTexture(textures[0]);
            sprites[0].setScale(1.9,1.9);

            // --> Cargamos las fichas
            for (int i = 1; i < 5; ++i){
                sprites[i].setTexture(textures[i]);
                sprites[i].setScale(0.12,0.12);
            }
        }

        void view(){
            window.clear();
            //draw the board
            window.draw(sprites[0]);
            int sprite_number;
            //draw the pawns
            for(const auto pawn_ptr: game_board.pawn_vector){
                if (auto drawn_pawn = pawn_ptr.lock()){
                    // --> player turn
                    sprite_number = drawn_pawn->owner == personPlayer ? 1 : 2;
                    // --> set coordinates of play and drawn
                    sprites[sprite_number].setPosition(drawn_pawn->x, drawn_pawn->y);
                    window.draw(sprites[sprite_number]);
                }
            }
            window.display();
        }
        void inicio(){
            window.create(sf::VideoMode(board_size, board_size), "Juego de las Damas");

            window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
            view();
        }

        //Computer Play
        int alphaBeta(Tablero_Class& current_board, Movimiento& best_move, int depth, TipoJugador player, int alpha, int beta){
            int value;
            // --> Caso Base
            if (depth == 0){
                value = current_board.getScore(IAPlayer) - current_board.getScore(personPlayer);
                return value;
            }

            std::vector<Movimiento>* possible_moves = current_board.getAvailibleMoves(player);
            std::vector<Tablero_Class>* possible_boards = new std::vector<Tablero_Class>(possible_moves->size(), current_board);

            for (unsigned int i = 0; i < possible_moves->size(); ++i){
                possible_boards->at(i).movePawn(possible_moves->at(i));
            }
            
            if (player == IAPlayer){
                for (unsigned int i = 0; i < possible_boards->size(); ++i){
                    value = alphaBeta(possible_boards->at(i), best_move, depth-1, personPlayer, alpha, beta);
                    alpha = std::max(alpha, value);
                    if (alpha == value && depth == 6)
                        best_move = possible_moves->at(i);
                    if (alpha >= beta){
                        // --> Poda alfa
                        break;
                    }
                }
                return alpha;
            }
            else{
                for (unsigned int i = 0; i < possible_boards->size(); ++i){
                    beta = std::min(beta, alphaBeta(possible_boards->at(i), best_move, depth-1, IAPlayer, alpha, beta));
                    if (alpha >= beta){
                        // --> Poda beta
                        break;
                    }
                }
                return beta;
            }
            delete possible_moves;
            delete possible_boards;
            // std::cerr << "stop poziom " << depth << "- " << value << '\n';
        }

        void executeMove(sf::Vector2i& start, sf::Vector2i& finish, TipoMovimiento type){
            if(auto pawn = game_board.movePawn(start, finish, type)){
                float distance_x = ((finish.x - start.x) * field_size) / 10;
                float distance_y = ((finish-start).y * field_size) / 10;
                for (int i = 0; i < 10; ++i){
                    pawn->x += distance_x;
                    pawn->y -= distance_y;
                    delay(20);
                    view();
                }
                view();
            }
        }
        
        int computerMove(){
            Movimiento computer_move;
            sf::Clock clock;
            clock.restart();

            // --> Alpha beta con profundidad 6
            alphaBeta(game_board, computer_move, 6, IAPlayer, INF_Negativo, INF_Positivo);
            cerr << clock.getElapsedTime().asMilliseconds();
            executeMove(computer_move.inicio, computer_move.fin, computer_move.tipo);
            return 0;
        }

        //User Play
        bool pollEvents(sf::Vector2i& mouse_position){
            sf::Event event;
            while (window.pollEvent(event)){
                if (event.type == sf::Event::Closed){
                    window.close();
                    return false;
                }
                if (event.type == sf::Event::MouseButtonPressed){
                    if (event.mouseButton.button == sf::Mouse::Left){
                        mouse_position.x = event.mouseButton.x;
                        mouse_position.y = event.mouseButton.y;
                        return true;
                    }
                }
            }
            return false;
        }
        
        int manualMove(TipoJugador player){
            sf::Vector2i mouse_position, start, finish;
            sf::Vector2i* updated_vector;
            std::shared_ptr<Ficha> active_pawn;
            bool mouse_pressed=false;
            while (window.isOpen()){
                mouse_pressed = pollEvents(mouse_position);
                if (mouse_pressed){
                    if(mouse_position.x > border_size && mouse_position.x < board_size - border_size &&
                    mouse_position.y > border_size && mouse_position.y < board_size - border_size){
                        if (!active_pawn){
                            updated_vector = &start;
                        }
                        else{
                            updated_vector = &finish;
                        }
                        updated_vector->x = (mouse_position.x - border_size) / field_size;
                        updated_vector->y = (mouse_position.y - border_size) / field_size;
                        updated_vector->y = 7 - updated_vector->y;
                        if (active_pawn){
                            //std::cerr << start.x << start.y << '-' << finish.x << finish.y << '\n';
                            if(active_pawn->owner == player){
                                TipoMovimiento result = game_board.checkMove(start, finish);
                                if (result != INVALID){
                                    executeMove(start, finish, result);
                                    return 0;
                                }
                            }
                            active_pawn = nullptr;
                        }
                        else {
                            active_pawn = game_board.getPawn(start);
                        }
                    }
                }
            }
            return 1;
        }

        int getMove(TipoJugador player){
            //Depend on the turn in the play
            game_board.resolveBeating(player);
            if (player == IAPlayer)
                return computerMove();
            else
                return manualMove(personPlayer);
        }

        void jugar(){
            //Start interactive play
            Movimiento IAPlayer_move;
            TipoJugador winner = winnerPlayer;
            while(winner == winnerPlayer){
                if(getMove(active_player))
                    break;
                // std::cerr << alphaBeta(game_board, IAPlayer_move, 2, COMPUTER, minus_infty, plus_infty);
                active_player = otherPlayer(active_player);
                winner = game_board.checkWin(active_player);
            }

            //Print the status finish game
            

            if (winner == personPlayer)
            {
                
                std::cout << "\n\n";
                std::cout << "                   .-=========-.\n";
                std::cout << "                   \\'-=======-'/\n";
                std::cout << "                   _|    .=.  |_\n";
                std::cout << "                  ((|  {{1}}  |))\n";
                std::cout << "                   \\|   /|\\   |/\n";
                std::cout << "                    \\__ '`' __/\n";
                std::cout << "                      _`) (`_\n";
                std::cout << "                    _/_______\\_\n";
                std::cout << "                   /___________\\\n\n";


                std::cout << "             You are the WINNER!!! \\O_0/\n";
            }
            else if (winner == IAPlayer)
                std::cout << "You are the LOOOSER :0 \n";
        }
};