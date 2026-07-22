#include <iostream>
#include <chess.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <cmath>

using namespace std;
using namespace chess;

#define int long long

const int INF = (int)1e18;
const int CHECKMATE = 1000000;
const int MATE_BOUND = CHECKMATE - 1000;
const int MAX_PLY = 128;
const int PLAY_DEPTH = 8;

const string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const int16_t MO_TT = 30000;
const int16_t MO_CAP = 20000;
const int16_t MO_KILLER1 = 9000;
const int16_t MO_KILLER2 = 8999;

const int PIECE_VAL[6] = {100, 320, 330, 500, 900, 20000};
const int MVV_VAL[6] = {100, 300, 300, 500, 900, 20000};

const int DOUBLED_PAWN_PENALTY = 12;
const int ISOLATED_PAWN_PENALTY = 10;
const int BISHOP_PAIR_BONUS = 30;
const int PASSED_PAWN_BONUS[8] = {0, 5, 10, 20, 35, 60, 100, 0};

const int FUTILITY_MARGIN_1 = 150;
const int FUTILITY_MARGIN_2 = 300;
const int DELTA_MARGIN = 200;

const int NULL_MOVE_MIN_DEPTH = 3;
const int LMR_MIN_DEPTH = 3;
const int LMR_MOVE_THRESHOLD = 4;

// Piece-square tables
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

int evaluate(Board &board) {
    int npm = 0;
    int score = 0;
    int whiteKingSq = -1, blackKingSq = -1;
    int whiteBishops = 0, blackBishops = 0;

    int whitePawnFile[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int blackPawnFile[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int whitePawnMaxRank[8];
    int blackPawnMinRank[8];

    for (int f = 0; f < 8; f++) {
        whitePawnMaxRank[f] = -1;
        blackPawnMinRank[f] = 8;
    }

    for (int i = 0; i < 64; i++) {
        Piece piece = board.at(Square(i));
        if (piece == Piece::NONE)
            continue;

        int t = (int)piece.type();

        if (t == 5) {
            if (piece.color() == Color::WHITE)
                whiteKingSq = i;
            else
                blackKingSq = i;
            continue;
        }

        if (t != 0)
            npm += PIECE_VAL[t];

        if (t == 2) {
            if (piece.color() == Color::WHITE)
                whiteBishops++;
            else
                blackBishops++;
        }

        if (t == 0) {
            int file = i % 8;
            int rank = i / 8;
            if (piece.color() == Color::WHITE) {
                whitePawnFile[file]++;
                whitePawnMaxRank[file] = max(whitePawnMaxRank[file], rank);
            } else {
                blackPawnFile[file]++;
                blackPawnMinRank[file] = min(blackPawnMinRank[file], rank);
            }
        }

        int v = PIECE_VAL[t];
        int pos = (piece.color() == Color::WHITE) ? PST[t][i ^ 56] : PST[t][i];

        if (piece.color() == Color::WHITE)
            score += v + pos;
        else
            score -= v + pos;
    }

    for (int f = 0; f < 8; f++) {
        if (whitePawnFile[f] > 1)
            score -= (whitePawnFile[f] - 1) * DOUBLED_PAWN_PENALTY;
        if (blackPawnFile[f] > 1)
            score += (blackPawnFile[f] - 1) * DOUBLED_PAWN_PENALTY;

        if (whitePawnFile[f] > 0) {
            bool isolated = (f == 0 || whitePawnFile[f - 1] == 0) &&
                             (f == 7 || whitePawnFile[f + 1] == 0);
            if (isolated)
                score -= ISOLATED_PAWN_PENALTY;
        }

        if (blackPawnFile[f] > 0) {
            bool isolated = (f == 0 || blackPawnFile[f - 1] == 0) &&
                             (f == 7 || blackPawnFile[f + 1] == 0);
            if (isolated)
                score += ISOLATED_PAWN_PENALTY;
        }
    }

    for (int f = 0; f < 8; f++) {
        if (whitePawnFile[f] == 0)
            continue;

        int rank = whitePawnMaxRank[f];
        bool passed = true;

        for (int nf = max(0LL, f - 1); nf <= min(7LL, f + 1); nf++)
            if (blackPawnMinRank[nf] > rank && blackPawnMinRank[nf] != 8)
                passed = false;

        if (passed)
            score += PASSED_PAWN_BONUS[rank];
    }

    for (int f = 0; f < 8; f++) {
        if (blackPawnFile[f] == 0)
            continue;

        int rank = blackPawnMinRank[f];
        bool passed = true;

        for (int nf = max(0LL, f - 1); nf <= min(7LL, f + 1); nf++)
            if (whitePawnMaxRank[nf] < rank && whitePawnMaxRank[nf] != -1)
                passed = false;

        if (passed)
            score -= PASSED_PAWN_BONUS[7 - rank];
    }

    if (whiteBishops >= 2)
        score += BISHOP_PAIR_BONUS;
    if (blackBishops >= 2)
        score -= BISHOP_PAIR_BONUS;

    auto kingScore = [&](int sq, bool isWhite) -> int {
        int idx = isWhite ? (sq ^ 56) : sq;
        if (npm >= 4000)
            return PST_KING_MG[idx];
        if (npm <= 2500)
            return PST_KING_EG[idx];
        double r = (npm - 2500.0) / (4000.0 - 2500.0);
        return (int)(r * PST_KING_MG[idx] + (1.0 - r) * PST_KING_EG[idx]);
    };

    if (whiteKingSq != -1)
        score += PIECE_VAL[5] + kingScore(whiteKingSq, true);
    if (blackKingSq != -1)
        score -= PIECE_VAL[5] + kingScore(blackKingSq, false);

    if (board.sideToMove() == Color::BLACK)
        score = -score;

    return score;
}

enum Flag { EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    Move bestMove;
    Flag flag;
};

constexpr size_t TT_MAX_ENTRIES = 4000000;

class Chessbot {
public:
    Move killer[MAX_PLY][2];
    int history[2][64][64] = {};
    unordered_map<uint64_t, TTEntry> TT;

    int nodes = 0;
    bool stopped = false;

    chrono::steady_clock::time_point searchStart;
    int budgetMs = 0;
    bool timed = false;

    Chessbot() {
        TT.reserve(1 << 20);
        clearKillers();
    }

    void clearKillers() {
        for (int i = 0; i < MAX_PLY; i++) {
            killer[i][0] = Move::NULL_MOVE;
            killer[i][1] = Move::NULL_MOVE;
        }
    }

    void newSearch(int budget, bool isTimed){
        nodes=0;
        stopped=false;
        budgetMs=budget;
        timed=isTimed;
        clearKillers();
        searchStart=chrono::steady_clock::now();
    }

    int elapsedMs() const {
        return (int)chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - searchStart).count();
    }

    void checkTime() {
        if (timed && (nodes & 255) == 0 && elapsedMs() >= budgetMs)
            stopped = true;
    }

    int mvvLva(Board &board, const Move &m) const {
        int attackerType = (int)board.at(m.from()).type();
        int victimType = (m.typeOf() == Move::ENPASSANT) ? 0 : (int)board.at(m.to()).type();
        return MVV_VAL[victimType] * 10 - PIECE_VAL[attackerType];
    }

    bool hasNonPawnMaterial(Board &board, Color side) {
        for (int i = 0; i < 64; i++) {
            Piece piece = board.at(Square(i));
            if (piece == Piece::NONE)
                continue;
            if (piece.color() != side)
                continue;

            int t = (int)piece.type();
            if (t != 0 && t != 5)
                return true;
        }
        return false;
    }

    void orderMoves(Movelist &moves, Board &board, int ply, Move ttMove) {
        for (auto &move : moves) {
            if (move == ttMove) {
                move.setScore(MO_TT);
                continue;
            }

            if (board.isCapture(move)) {
                move.setScore(MO_CAP + mvvLva(board, move));
                continue;
            }

            if (move.typeOf() == Move::PROMOTION) {
                move.setScore(MO_CAP);
                continue;
            }

            if (move == killer[ply][0]) {
                move.setScore(MO_KILLER1);
                continue;
            }

            if (move == killer[ply][1]) {
                move.setScore(MO_KILLER2);
                continue;
            }

            int color = (board.sideToMove() == Color::WHITE);
            move.setScore(history[color][move.from().index()][move.to().index()]);
        }

        sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) { return a.score() > b.score(); });
    }

    int quiescence(Board &board, int alpha, int beta, int ply) {
        nodes++;
        checkTime();
        if (stopped)
            return 0;

        if (ply >= MAX_PLY - 4)
            return evaluate(board);

        if (board.isHalfMoveDraw()) {
            auto res = board.getHalfMoveDrawType();
            return (res.first == GameResultReason::CHECKMATE) ? -(CHECKMATE - ply) : 0;
        }
        if (board.isRepetition(1))
            return 0;
        if (board.isInsufficientMaterial())
            return 0;

        bool inCheckNode = board.inCheck();
        Movelist moves;
        int best;
        int stand = 0;

        if (inCheckNode) {
            movegen::legalmoves(moves, board);
            if (moves.empty())
                return -(CHECKMATE - ply);
            best = -INF;
        } else {
            stand = evaluate(board);
            if (stand >= beta)
                return stand;
            if (stand > alpha)
                alpha = stand;
            best = stand;
            movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
        }

        orderMoves(moves, board, ply, Move::NULL_MOVE);

        for (int moveNo = 0; moveNo < (int)moves.size(); moveNo++) {
            Move move = moves[moveNo];

            if (!inCheckNode) {
                int victimType = (move.typeOf() == Move::ENPASSANT) ? 0 : (int)board.at(move.to()).type();
                if (stand + PIECE_VAL[victimType] + DELTA_MARGIN < alpha)
                    continue;
            }

            board.makeMove(move);
            int score = -quiescence(board, -beta, -alpha, ply + 1);
            board.unmakeMove(move);

            if (stopped)
                return 0;

            if (score > best)
                best = score;
            if (score > alpha)
                alpha = score;
            if (alpha >= beta)
                break;
        }

        return best;
    }

    int negamax(Board &board, int depth, int ply, int alpha, int beta, vector<Move> &pv) {
        pv.clear();

        nodes++;
        checkTime();
        if (stopped)
            return 0;

        if (ply >= MAX_PLY - 4)
            return evaluate(board);

        if (board.isHalfMoveDraw()) {
            auto res = board.getHalfMoveDrawType();
            return (res.first == GameResultReason::CHECKMATE) ? -(CHECKMATE - ply) : 0;
        }
        if (board.isRepetition(1))
            return 0;
        if (board.isInsufficientMaterial())
            return 0;

        bool inCheck = board.inCheck();
        if (inCheck)
            depth++;

        int alphaOrig = alpha;
        uint64_t key = board.hash();
        Move ttMove = Move::NULL_MOVE;

        auto it = TT.find(key);
        if (it != TT.end()) {
            TTEntry &e = it->second;
            ttMove = e.bestMove;

            if (e.depth >= depth && ply > 0) {
                int s = e.score;
                if (s > MATE_BOUND)
                    s -= ply;
                if (s < -MATE_BOUND)
                    s += ply;

                if (e.flag == EXACT) {
                    pv = {e.bestMove};
                    return s;
                }
                if (e.flag == LOWERBOUND)
                    alpha = max(alpha, s);
                else
                    beta = min(beta, s);

                if (alpha >= beta) {
                    pv = {e.bestMove};
                    return s;
                }
            }
        }

        if (depth <= 0)
            return quiescence(board, alpha, beta, ply);

        if (!inCheck && depth >= NULL_MOVE_MIN_DEPTH && ply > 0 &&
            beta < MATE_BOUND && hasNonPawnMaterial(board, board.sideToMove())) {
            int reduction = (depth >= 6) ? 3 : 2;
            vector<Move> nullPv;

            board.makeNullMove();
            int nullScore = -negamax(board, depth - 1 - reduction, ply + 1, -beta, -beta + 1, nullPv);
            board.unmakeNullMove();

            if (stopped)
                return 0;

            if (nullScore >= beta)
                return beta;
        }

        Movelist moves;
        movegen::legalmoves(moves, board);

        if (moves.empty())
            return inCheck ? -(CHECKMATE - ply) : 0;

        orderMoves(moves, board, ply, ttMove);

        bool canFutilityPrune = false;
        if (!inCheck && depth <= 2 && llabs(alpha) < MATE_BOUND && llabs(beta) < MATE_BOUND) {
            int margin = (depth == 1) ? FUTILITY_MARGIN_1 : FUTILITY_MARGIN_2;
            canFutilityPrune = (evaluate(board) + margin <= alpha);
        }

        Move bestMove = Move::NULL_MOVE;
        int best = -INF;
        vector<Move> childPv;

        for (int moveNo = 0; moveNo < (int)moves.size(); moveNo++) {
            Move move = moves[moveNo];
            bool isCapture = board.isCapture(move);
            bool isPromotion = move.typeOf() == Move::PROMOTION;
            bool isKiller = (move == killer[ply][0] || move == killer[ply][1]);

            if (canFutilityPrune && moveNo > 0 && !isCapture && !isPromotion)
                continue;

            board.makeMove(move);

            int score;

            if (moveNo == 0) {
                score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, childPv);
            } else {
                int reduction = 0;
                if (depth >= LMR_MIN_DEPTH && moveNo >= LMR_MOVE_THRESHOLD &&
                    !isCapture && !isPromotion && !isKiller && !inCheck) {
                    reduction = (moveNo >= 10) ? 2 : 1;
                }

                score = -negamax(board, depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, childPv);

                if (score > alpha && reduction > 0)
                    score = -negamax(board, depth - 1, ply + 1, -alpha - 1, -alpha, childPv);

                if (score > alpha && score < beta)
                    score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, childPv);
            }

            board.unmakeMove(move);

            if (stopped)
                return 0;

            if (score > best) {
                best = score;
                bestMove = move;
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPv.begin(), childPv.end());
            }

            if (best > alpha)
                alpha = best;

            if (alpha >= beta) {
                if (!isCapture) {
                    killer[ply][1] = killer[ply][0];
                    killer[ply][0] = move;
                    int color = (board.sideToMove() == Color::WHITE);
                    int &h = history[color][move.from().index()][move.to().index()];
                    h += depth * depth;
                    if (h > 16384)
                        h = 16384;
                }
                break;
            }
        }

        if (stopped)
            return 0;

        Flag flag;
        if (best <= alphaOrig)
            flag = UPPERBOUND;
        else if (best >= beta)
            flag = LOWERBOUND;
        else
            flag = EXACT;

        int stored = best;
        if (stored > MATE_BOUND)
            stored += ply;
        if (stored < -MATE_BOUND)
            stored -= ply;

        auto existing = TT.find(key);
        if (existing == TT.end() || existing->second.depth <= depth) {
            if (existing != TT.end()) {
                existing->second = {key, depth, stored, bestMove, flag};
            } else if (TT.size() < TT_MAX_ENTRIES) {
                TT.emplace(key, TTEntry{key, depth, stored, bestMove, flag});
            }
        }

        return best;
    }

    Move bestMove(Board &board, int maxDepth) {
        Move best = Move::NULL_MOVE;
        vector<Move> pv;
        int lastIterationTime = 0;
        int lastScore = 0;

        {
            Movelist ml;
            movegen::legalmoves(ml, board);
            if (!ml.empty())
                best = ml[0];
        }

        for (int depth = 1; depth <= maxDepth; depth++) {
            if (timed) {
                if (elapsedMs() >= budgetMs)
                    break;

                if (depth > 1 && elapsedMs() + lastIterationTime * 1.5 >= budgetMs)
                    break;
            }

            int iterStart = elapsedMs();
            vector<Move> iterPv;
            int score;

            if (depth <= 2) {
                score = negamax(board, depth, 0, -INF, INF, iterPv);
            } else {
                int window = 35;

                while (true) {
                    int alpha = max(-INF, lastScore - window);
                    int beta = min(INF, lastScore + window);

                    score = negamax(board, depth, 0, alpha, beta, iterPv);

                    if (stopped)
                        break;

                    if (score <= alpha || score >= beta) {
                        window *= 2;
                        continue;
                    }

                    break;
                }
            }

            if (stopped)
                break;

            lastIterationTime = elapsedMs() - iterStart;
            lastScore = score;

            if (!iterPv.empty())
                best = iterPv[0];

            pv = iterPv;

            cout << "info depth " << depth
                 << " score " << uciScore(score)
                 << " nodes " << nodes
                 << " time " << elapsedMs()
                 << " pv";

            for (auto &m : pv)
                cout << " " << uci::moveToUci(m);

            cout << "\n" << flush;

            if (llabs(score) > MATE_BOUND)
                break;
        }

        return best;
    }

    static string uciScore(int score) {
        if (llabs(score) > MATE_BOUND) {
            int plies = CHECKMATE - llabs(score);
            int moves = (plies + 1) / 2;
            return "mate " + to_string(score > 0 ? moves : -moves);
        }
        return "cp " + to_string(score);
    }
};

int computeBudget(int myTime, int myInc, int movesToGo){
    myInc = (3* myInc)/4.0;
    if (myTime <= 0)
        return 5;

    // GUI knows roughly how many moves remain
    if (movesToGo > 0) {
        int budget = myTime / (movesToGo + 1) + myInc;

        budget = min(budget, myTime / 3);
        budget = max(5LL, budget);

        return budget;
    }

    int budget;

    if (myTime > 60000)
        budget = myTime / 25 + myInc;
    else if (myTime > 30000)
        budget = myTime / 20 + myInc;
    else if (myTime > 10000)
        budget = myTime / 14 + myInc;
    else if (myTime > 5000)
        budget = myTime / 10 + myInc;
    else if (myTime > 2000)
        budget = myTime / 8 + myInc;
    else
        budget = max(40LL, myTime / 6);

    budget = min(budget, myTime / 3);
    budget = max(5LL, budget);

    return budget;
}

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Chessbot bot;
    Board board(START_FEN);

    vector<string> appliedMoves;
    string lastFen = START_FEN;

    string line;
    while (getline(cin, line)) {
        if (line.empty())
            continue;

        if (line == "uci") {
            cout << "id name NunnuBot\n";
            cout << "id author Majid\n";
            cout << "uciok\n" << flush;
        } else if (line == "isready") {
            cout << "readyok\n" << flush;
        } else if (line == "ucinewgame") {
            board = Board(START_FEN);
            appliedMoves.clear();
            lastFen = START_FEN;
            bot.TT.clear();
            bot.clearKillers();
        } else if (line.rfind("position", 0) == 0) {
            stringstream ss(line);
            string token;
            ss >> token;
            ss >> token;

            string baseFen;
            if (token == "startpos") {
                baseFen = START_FEN;
            } else if (token == "fen") {
                for (int i = 0; i < 6; i++) {
                    ss >> token;
                    if (i)
                        baseFen += " ";
                    baseFen += token;
                }
            }

            vector<string> moveTokens;
            while (ss >> token) {
                if (token == "moves")
                    continue;
                moveTokens.push_back(token);
            }

            bool isExtension = (baseFen == lastFen) &&
                                (moveTokens.size() >= appliedMoves.size());
            if (isExtension) {
                for (size_t i = 0; i < appliedMoves.size(); i++) {
                    if (moveTokens[i] != appliedMoves[i]) {
                        isExtension = false;
                        break;
                    }
                }
            }

            size_t startIdx;
            if (isExtension) {
                startIdx = appliedMoves.size();
            } else {
                board = Board(baseFen);
                lastFen = baseFen;
                appliedMoves.clear();
                startIdx = 0;
            }

            for (size_t i = startIdx; i < moveTokens.size(); i++) {
                Movelist ml;
                movegen::legalmoves(ml, board);
                for (auto &m : ml) {
                    if (uci::moveToUci(m) == moveTokens[i]) {
                        board.makeMove(m);
                        break;
                    }
                }
            }
            appliedMoves = moveTokens;
        } else if (line.rfind("go", 0) == 0) {
            stringstream ss(line);
            string token;
            ss >> token;

            int wtime = -1, btime = -1;
            int winc = 0, binc = 0;
            int movetime = -1;
            int depth = -1;
            int movestogo = -1;
            bool infinite = false;

            while (ss >> token) {
                if (token == "wtime")
                    ss >> wtime;
                else if (token == "btime")
                    ss >> btime;
                else if (token == "winc")
                    ss >> winc;
                else if (token == "binc")
                    ss >> binc;
                else if (token == "movetime")
                    ss >> movetime;
                else if (token == "depth")
                    ss >> depth;
                else if (token == "movestogo")
                    ss >> movestogo;
                else if (token == "infinite")
                    infinite = true;
            }

            int budget = 0;
            bool isTimed = true;
            int maxDepth = MAX_PLY;

            if (movetime > 0) {
                budget = movetime - 30;
                if (budget < 0)
                    budget = 0;
            } else if (!infinite && (wtime >= 0 || btime >= 0)) {
                bool white = (board.sideToMove() == Color::WHITE);
                int myTime = white ? wtime : btime;
                int myInc = white ? winc : binc;
                if (myTime < 0)
                    myTime = 0;
                budget = computeBudget(myTime, myInc, movestogo);
            } else {
                isTimed = false;
                maxDepth = (depth > 0) ? depth : PLAY_DEPTH;
                budget = 0;
            }

            if (depth > 0)
                maxDepth = min(maxDepth, depth);
            if (isTimed && budget < 5)
                budget = 5;

            bot.newSearch(budget, isTimed);
            Move best = bot.bestMove(board, maxDepth);

            if (best == Move::NULL_MOVE) {
                Movelist ml;
                movegen::legalmoves(ml, board);
                if (!ml.empty())
                    best = ml[0];
            }

            cout << "bestmove " << uci::moveToUci(best) << "\n" << flush;
        } else if (line == "quit") {
            break;
        }
    }

    return 0;
}