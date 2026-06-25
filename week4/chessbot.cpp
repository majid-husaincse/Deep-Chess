#include<iostream>
#include<chess.hpp>
#include<string>
#include<vector>
#include<algorithm>
#include<sstream>
#include<unordered_map>
constexpr int TT_SCORE = 30000;
constexpr int KILLER1  = 29000;
constexpr int KILLER2  = 28999;
using namespace std;
using namespace chess;
enum Flag
{
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TTEntry
{
    uint64_t hash;
    long long depth;
    long long score;
    Move bestMove;
    Flag flag;
};

#define int long long
const int INF = 1e9;
const int CHECKMATE = 1e7;
const int MAX_PLY = 128;
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
        if(board.sideToMove() == Color::BLACK)
        score = -score;
        return score;
        }
bool compare(const Move &a,const Move &b){
        return a.score() > b.score();
        }


class chessbot{

    public:
    Move killer[MAX_PLY][2];
    unordered_map<uint64_t, TTEntry> TT;

    // constructer
    chessbot(){
        TT.reserve(1<< 20);
    }


void ordermoves(Movelist &moves, Board &board, int ply, Move ttMove)
{
    for(auto &move : moves)
    {
        if(move == ttMove){
            move.setScore(TT_SCORE);
            continue;
        }

        if(move == killer[ply][0]){
            move.setScore(KILLER1);
            continue;
        }

        if(move == killer[ply][1]){
            move.setScore(KILLER2);
            continue;
        }

        int score = 0;

if(board.isCapture(move) && move.typeOf() != Move::CASTLING){
    int victimValue = (move.typeOf() == Move::ENPASSANT)
                        ? PieceValue(PieceType::PAWN)
                        : PieceValue(board.at(move.to()).type());
    Piece attacker = board.at(move.from());
    score = 10 * victimValue - PieceValue(attacker.type());
}

        if(board.givesCheck(move) != CheckType::NO_CHECK)
            score += 10000;

        move.setScore(score);
    }

    sort(moves.begin(), moves.end(), compare);
}

int quiescence(Board &board, int alpha, int beta, int ply)
{
    if(ply >= MAX_PLY - 4)
        return evaluate(board);

    if(board.isHalfMoveDraw())
    {
        auto type = board.getHalfMoveDrawType();
        return type.first == GameResultReason::CHECKMATE ? -(CHECKMATE - ply) : 0;
    }
    if(board.isRepetition(1))
        return 0;

    Movelist moves;

    if(board.inCheck())
    {
        movegen::legalmoves(moves, board);

        if(moves.empty())
            return -(CHECKMATE - ply);
    }
    else
    {
        int stand = evaluate(board);

        if(stand >= beta)
            return stand;

        if(stand > alpha)
            alpha = stand;

        movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    }

    ordermoves(moves, board,ply,Move::NULL_MOVE);

    int best = alpha;
    for(auto &move : moves)
    {
        board.makeMove(move);

        int score = -quiescence(board,-beta,-alpha,ply + 1);
        board.unmakeMove(move);

        if(score > best)
            best = score;
        if(score > alpha)
            alpha = score;
        if(alpha >= beta)
            return score;      
    }

    return best;
}

pair<int,Move> negamax(Board &board,int depth,int ply,int alpha,int beta)
{
    if(board.isHalfMoveDraw())
    {
        auto type = board.getHalfMoveDrawType();
        return {type.first == GameResultReason::CHECKMATE ? -(CHECKMATE - ply) : 0, Move::NULL_MOVE};
    }
    if(board.isRepetition(1))
        return {0, Move::NULL_MOVE};

    int alphaOrig = alpha;
    int betaOrig = beta;

    uint64_t key = board.hash();
    Move ttMove = Move::NULL_MOVE;

    auto it = TT.find(key);
    if(it != TT.end())
    {
        TTEntry &entry = it->second;
        ttMove = entry.bestMove;

        if(entry.depth >= depth)
        {
            int ttScore = entry.score;
            if(ttScore > CHECKMATE - 1000) ttScore -= ply;
            else if(ttScore < -(CHECKMATE - 1000)) ttScore += ply;

            if(entry.flag == EXACT)
                return {ttScore, entry.bestMove};

            if(entry.flag == LOWERBOUND)
                alpha = max(alpha, ttScore);
            else if(entry.flag == UPPERBOUND)
                beta = min(beta, ttScore);

            if(alpha >= beta)
                return {ttScore, entry.bestMove};
        }
    }

        if(depth <= 0)
        return {quiescence(board,alpha,beta,ply),Move::NULL_MOVE};

    Movelist moves;
    movegen::legalmoves(moves,board);

    if(moves.empty()){
        if(!board.inCheck())
            return {0,Move::NULL_MOVE};

        return {-(CHECKMATE-ply),Move::NULL_MOVE};
    }

    ordermoves(moves, board, ply, ttMove);

    Move bestMove = Move::NULL_MOVE;

    for(auto &move : moves)
    {
        board.makeMove(move);
        auto child = negamax(board,depth-1,ply+1,-beta,-alpha);
        board.unmakeMove(move);

        int score = -child.first;

        if(score > alpha){
            alpha = score;
            bestMove = move;
        }

        if(alpha >= beta){

            if(!board.isCapture(move)){
                killer[ply][1] = killer[ply][0];
                killer[ply][0] = move;
            }

            int storeScore = score;
            if(storeScore > CHECKMATE - 1000) storeScore += ply;
            else if(storeScore < -(CHECKMATE - 1000)) storeScore -= ply;

            TT[key] = {key, depth, storeScore, move, LOWERBOUND};
            return {score, move};
        }
    }

    Flag flag;

    if(alpha <= alphaOrig)
        flag = UPPERBOUND;
    else if(alpha >= betaOrig)
        flag = LOWERBOUND;
    else
        flag = EXACT;

    int storeScore = alpha;
    if(storeScore > CHECKMATE - 1000) storeScore += ply;
    else if(storeScore < -(CHECKMATE - 1000)) storeScore -= ply;

    TT[key] = {key, depth, storeScore, bestMove, flag};

    return {alpha, bestMove};
}

Move makebestmove(Board &board,int depth){

    Move best = Move::NULL_MOVE;

    for(int i=1;i<=depth;i++)
        best = negamax(board,i,0,-INF,INF).second;
    return best;
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

else if(line == "ucinewgame")
{
    board = Board(START_FEN);

    Bot.TT.clear();

    for(int i=0;i<MAX_PLY;i++)
    {
        Bot.killer[i][0] = Move::NULL_MOVE;
        Bot.killer[i][1] = Move::NULL_MOVE;
    }
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

else if(line.starts_with("position fen"))
{
    stringstream ss(line);

    string token;
    ss >> token; // position
    ss >> token; // fen

    string fen;
    for(int i = 0; i < 6; i++)
    {
        ss >> token;
        fen += token;
        if(i < 5) fen += " ";
    }

    board = Board(fen);

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