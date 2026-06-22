#include "chess.hpp"
#include<iostream>
#include<string>
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
const int inf = 1e9;
const int CHECKMATE = 1e6;
using namespace chess;
int PieceValue (PieceType x){
    using PT = PieceType;
    auto type = x.internal();
    if(type == PT::PAWN){
        return 100;
    }
    if(type == PT::KNIGHT){
        return 305;
    }
    if(type == PT::BISHOP){
        return 333;
    }
    if(type == PT::ROOK){
        return 563;
    }
    if(type == PT::QUEEN){
        return 950;
    }
    if(type == PT::KING){
        return CHECKMATE;
    }
    return 0;
}

int evaluate(Board &board){
    int score = 0;
    for(int i=0; i<64; i++){
        Piece piece = board.at(Square(i));
        score += PieceValue(piece.type()) * (piece.color() == Color::WHITE ? 1 : -1);
    }
    return score;
}
bool compare(const Move &a,const Move &b){
        return a.score() > b.score();
}
void ordermoves(Movelist &moves,Board &board){
    for(auto &move : moves){
        int score = 0;
        if(board.isCapture(move)){
            Piece victim = board.at(move.to());
            Piece attacker = board.at(move.from());
            score = 10*PieceValue(victim.type()) - PieceValue(attacker.type());
        }
        if(board.givesCheck(move) != CheckType::NO_CHECK) score+=10000;
        move.setScore(score);
    }
    std::sort(moves.begin(), moves.end(), compare);
}

std::pair<int,Move> minimax(Board &board,int depth,int ply,int alpha,int beta,bool maximising){
    
    Movelist moves;
    movegen::legalmoves(moves,board);

    ordermoves(moves,board);
    if(moves.empty()){
        if( ! board.inCheck()){
            return {0,Move::NULL_MOVE};
        }
            return {-(2*maximising-1)*(CHECKMATE-ply),Move::NULL_MOVE};
        }
    if(depth == 0){
        return {evaluate(board),Move::NULL_MOVE};
    }
Move movehehe = Move::NULL_MOVE;
    for(auto &move : moves){
        board.makeMove(move);
        auto child = minimax(board,depth-1,ply+1,alpha,beta,1-maximising);
        if(maximising){
            if(child.first > alpha){
                alpha = child.first;
                movehehe = move;
            }
        }
        else{
            if(child.first < beta){
                beta = child.first;
                movehehe = move;
            }
        }
        board.unmakeMove(move);
        if(alpha>=beta){
            return {child.first,move};
        }
            }
        return {maximising ? alpha : beta,movehehe};
        }
int main(){

    std::ifstream f("mate_in_4.json");

    json puzzles;
    f >> puzzles;

    int total = 0;
    int solved = 0;

    auto start =
        std::chrono::high_resolution_clock::now();

    for(auto &[fen, answer] : puzzles.items()){

        total++;

        Board board(fen);

        bool maximising =
            (board.sideToMove() == Color::WHITE);

        auto result =
            minimax(board,
                    7,      // mate in 4
                    0,
                    -inf,
                    inf,
                    maximising);

        if(std::abs(result.first) > CHECKMATE - 1000)
            solved++;
            //std::cout<<solved;
    }

    auto end =
        std::chrono::high_resolution_clock::now();

    double secs =
        std::chrono::duration<double>(end-start).count();

    std::cout << "Total puzzles: "
              << total
              << "\n";

    std::cout << "Solved: "
              << solved
              << "\n";

    std::cout << "Accuracy: "
              << 100.0 * solved / total
              << "%\n";

    std::cout << "Time per puzzle: "
              << secs / total
              << " sec\n";

    std::cout << "Total time: "
              << secs
              << " sec\n";
}