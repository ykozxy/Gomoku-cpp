#ifndef GOMOKU_CONSTANT_H
#define GOMOKU_CONSTANT_H

#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>
#include <limits>
#include <unordered_map>

using namespace std;


#define IN_RANGE(a, b) assert((a)>=0 && (b)>=0 && (a)<BOARD_SIZE && (b)<BOARD_SIZE)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MS_DIFF(start, end) (std::chrono::duration_cast<std::chrono::milliseconds>((end) - (start)).count())

#ifndef assert
#define assert(a) ;
#endif

typedef std::chrono::high_resolution_clock Clock;


const int BOARD_SIZE = 15;
const int SCORE_RANGE = 5;
const int TIME_LIMIT = 990;

enum Chess {
    c_empty = -1, black = 0, white = 1
};

enum Direction {
    vertical = 0, horizontal = 1, diag_LU = 2, diag_RU = 3
};

enum Forms {
    _5 = 1000000, _4p = 10000, _4m = 1000, _3p = 1000, _3m = 100, _2p = 100, _2m = 10, _1p = 10, _1m = 1
};

struct Point {
    int x, y;
    int ai_score, op_score;

    Point() : x(-1), y(-1), ai_score(-1), op_score(-1) {}

    Point(const Point &p) = default;

    Point(int x, int y, int aiScore = -1, int opScore = -1) : x(x), y(y), ai_score(aiScore), op_score(opScore) {}
};

struct CacheData {
    int score, depth;

    explicit CacheData(int score, int depth) : score(score), depth(depth) {}
};


#endif //GOMOKU_CONSTANT_H
