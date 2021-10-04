#ifndef GOMOKU_MINIMAXAI_H
#define GOMOKU_MINIMAXAI_H

#include <string>

#include "constants.h"
#include "Board.h"

class MinimaxAI {
public:
    MinimaxAI(Board *board, Chess identity, float weight = 0.5, int pruneLimit = 20) :
            m_board(board), m_identity(identity), m_weight(weight), m_pruneLimit(pruneLimit), m_breakout(false) {}

    Point calculate(std::string *buff = nullptr);

private:
    float m_weight;
    bool m_breakout;
    int m_pruneLimit;
    Board *m_board;
    Chess m_identity;
    std::chrono::time_point<Clock> startT;

    int miniMaxWrapper(int depth, Point *candidates, int n);

    int miniMaxSearch(int depth, int alpha, int beta, Chess player, bool checkmateOnly);
};



#endif //GOMOKU_MINIMAXAI_H
