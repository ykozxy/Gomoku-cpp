#ifndef GOMOKU_CONSTANT_H
#define GOMOKU_CONSTANT_H

#include <chrono>

#define IN_RANGE(a, b) assert((a)>=0 && (b)>=0 && (a)<BOARD_SIZE && (b)<BOARD_SIZE)
#define MS_DIFF(start, end) (std::chrono::duration_cast<std::chrono::milliseconds>((end) - (start)).count())

#ifndef assert
#define assert(a) ;
#endif

typedef std::chrono::high_resolution_clock Clock;


const int BOARD_SIZE = 15;
const int SCORE_RANGE = 5;
const int TIME_LIMIT = 990;
// const int TIME_LIMIT = 5000;
const int MINIMAX_DEPTH = 8;
const int CHECKMATE_DEPTH = 4;


enum Chess {
    c_empty = -1, black = 0, white = 1
};

enum Direction {
    vertical = 0, horizontal = 1, diag_LU = 2, diag_RU = 3
};

enum Forms {
    _5 = 1000000,
    _4p = 10000,
    // _4p_spaced = 6000,
    _4m = 1001,
    _3p = 1000,
    // _3p_spaced = 600,
    _3m = 101,
    _2p = 100,
    _2p_spaced = 50,
    _2m = 10,
    _1p = 9,
    _1m = 1,
    _empty = 0
};

struct Point {
    short x, y;

    int ai_score, op_score;

    Point() : x(-1), y(-1), ai_score(-1), op_score(-1) {}

    Point(const Point &p) = default;

    Point(short x, short y, int aiScore = -1, int opScore = -1) : x(x), y(y), ai_score(aiScore), op_score(opScore) {}
};


struct MinimaxNode {
    int score, depth;
};

struct Coord {
    short x, y;

    Coord() : x(-1), y(-1) {};

    Coord(short r, short c) : x(r), y(c) {}
};

struct CacheData {
    MinimaxNode *minimaxNode;

    explicit CacheData(int score, int depth) {
        minimaxNode = new MinimaxNode;
        minimaxNode->score = score;
        minimaxNode->depth = depth;
    }

    ~CacheData() {
        delete minimaxNode;
    }
};


#endif //GOMOKU_CONSTANT_H
