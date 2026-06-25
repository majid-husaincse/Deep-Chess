#include<iostream>
#include<chess.hpp>
#include<string>
#include<vector>
#include<algorithm>
#include<sstream>

using namespace std;
using namespace chess;

#define int long long
const int INF = 1e9;
const int CHECKMATE = 1e7;
const string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const Board START_BOARD(START_FEN);

const int PIECE_VALUES[6] = {100, 320, 325, 500, 975, 20000}; // Relative piece values (Thanks to Adamberant)

// Piece tables (Thanks again to adambarent);

const int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};
const int PST_KNIGHT[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};
const int PST_BISHOP[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};
const int PST_ROOK[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};
const int PST_QUEEN[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
     0,  0,  5,  5,  5,  5,  0, -5,
   -10,  5,  5,  5,  5,  5,  0,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};
const int PST_KING_MG[64] = {
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};
const int PST_KING_EG[64] = {
   -50,-40,-30,-20,-20,-30,-40,-50,
   -30,-20,-10,  0,  0,-10,-20,-30,
   -30,-10, 20, 30, 30, 20,-10,-30,
   -30,-10, 30, 40, 40, 30,-10,-30,
   -30,-10, 30, 40, 40, 30,-10,-30,
   -30,-10, 20, 30, 30, 20,-10,-30,
   -30,-30,  0,  0,  0,  0,-30,-30,
   -50,-30,-30,-30,-30,-30,-30,-50
};

const int* PST[5] = {PST_PAWN, PST_KNIGHT, PST_BISHOP, PST_ROOK, PST_QUEEN};



class chessbot{

    public:

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

        int Motepieces = 0; // non pawns
        int score = 0;
        for(int i=0; i<64; i++){
        Piece piece = board.at(Square(i));
        if(piece == Piece::NONE) continue;
        int pieceType = (int)piece.type();
        if(pieceType !=0 && pieceType != 5) Motepieces+=PIECE_VALUES[pieceType];
        }
        for(int i=0;i<64;i++){
        Piece piece = board.at(Square(i));
        if(piece == Piece::NONE) continue;
        int pieceType = (int)piece.type();
        int x = PIECE_VALUES[pieceType];
        int position = 0;
        if(piece.color() == Color::WHITE){
        if(pieceType == 5){
            if(Motepieces > 4000) position = PST_KING_MG[i^56];
            else if(Motepieces < 2500) position = PST_KING_EG[i^56];
            else {
                double ratio = (Motepieces - 2500.0)/(4000.0-2500);
                position = ratio*(PST_KING_MG[i^56]) + (1 - ratio)*(PST_KING_EG[i^56]);
            }
        }
        else{
            position = PST[pieceType][i^56];
        }
        score+=x + position;
        }
        else{
            if(pieceType == 5){
            if(Motepieces > 4000) position = PST_KING_MG[i];
            else if(Motepieces < 2500) position = PST_KING_EG[i];
            else{
                double ratio = (Motepieces - 2500.0)/(4000.0-2500);
                position = ratio*(PST_KING_MG[i]) + (1 - ratio)*(PST_KING_EG[i]);
            }
        }
        else{
            position = PST[pieceType][i];
        }
        score-=x + position;
        }

        }
        return score;
        }

    static bool compare(const Move &a,const Move &b){
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
    Move makebestmove(Board &board,int depth){
        bool maximising = (board.sideToMove() == Color::WHITE);
        return (minimax(board,depth,0,-INF,INF,maximising).second);
    }
};

signed main() {

    chessbot Bot;
    Board board(START_FEN);

    string line;

    while(getline(cin, line)) {

        if(line == "uci") {
            cout << "id name NunnuBot\n";
            cout << "id author Majid\n";
            cout << "uciok\n";
        }

        else if(line == "isready") {
            cout << "readyok\n";
        }

        else if(line == "ucinewgame") {
            board = Board(START_FEN);
        }

else if(line.starts_with("position startpos"))
{
    board = Board(START_FEN);

    stringstream ss(line);

    string token;
    ss >> token; // position
    ss >> token; // startpos

    while(ss >> token)
    {
        if(token=="moves")
            continue;

        Movelist ml;
        movegen::legalmoves(ml,board);

        for(auto m:ml)
        {
            if(uci::moveToUci(m)==token)
            {
                board.makeMove(m);
                break;
            }
        }
    }
}

        else if(line.rfind("go",0) == 0) {

            Move best = Bot.makebestmove(board,7);

            cout << "bestmove "
                 << uci::moveToUci(best)
                 << "\n";
        }

        else if(line == "quit") {
            break;
        }
    }
}
