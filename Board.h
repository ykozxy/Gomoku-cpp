#ifndef GOMOKU_BOARD_H
#define GOMOKU_BOARD_H

#include <string>
#include <vector>
#include <unordered_map>
#include "constants.h"


class Board {
public:
    Board();

    /* Mutators */
    void set(int r, int c, Chess player);

    /* Accessors */
    [[nodiscard]] int getScore(Chess player) const;

    [[nodiscard]] int getScore(int r, int c, Chess player) const;

    [[nodiscard]] int getCount() const;

    [[nodiscard]] Chess getGrid(int r, int c) const;

    [[nodiscard]] bool hasEnd() const;

    [[nodiscard]] bool hasNeighbor(int r, int c, int range, int count) const;

    /* Heuristic */
    Point *heuristicGenerator(Chess player, Chess ai_id, int &resSize, bool checkmateOnly, bool do_sort);

    /* Cache */
    void cache(int score, int depth);

    [[nodiscard]] CacheData *getCache() const;

    [[nodiscard]] unsigned long getCachedSize() const;

    std::string to_string(std::vector<Point *> *planned = nullptr);

private:
    // Board
    Chess m_board[BOARD_SIZE][BOARD_SIZE]{};
    int m_neighborCount[2][BOARD_SIZE][BOARD_SIZE]{};
    int m_numChess = 0;
    bool m_win = false;

    // Scores
    int m_totalScore[2]{};
    Forms m_pointScores[2][4][BOARD_SIZE][BOARD_SIZE]{};

    // Caches
    long m_zobristCode;
    long m_zobristTable[2][BOARD_SIZE][BOARD_SIZE]{};
    std::unordered_map<long, CacheData *> m_cache;

    // Heuristic
    Point ai_5[BOARD_SIZE * BOARD_SIZE / 3], op_5[BOARD_SIZE * BOARD_SIZE / 3],
            ai_4p[BOARD_SIZE * BOARD_SIZE / 3], op_4p[BOARD_SIZE * BOARD_SIZE / 3],
            ai_combo[BOARD_SIZE * BOARD_SIZE / 3], op_combo[BOARD_SIZE * BOARD_SIZE / 3],
            ai_double3[BOARD_SIZE * BOARD_SIZE / 3], op_double3[BOARD_SIZE * BOARD_SIZE / 3],
            ai_4m[BOARD_SIZE * BOARD_SIZE / 3], op_4m[BOARD_SIZE * BOARD_SIZE / 3],
            ai_3p[BOARD_SIZE * BOARD_SIZE / 3], op_3p[BOARD_SIZE * BOARD_SIZE / 3],
            ai_2p[BOARD_SIZE * BOARD_SIZE / 3], op_2p[BOARD_SIZE * BOARD_SIZE / 3],
            neighbor[BOARD_SIZE * BOARD_SIZE / 3], res_point[BOARD_SIZE * BOARD_SIZE / 3];

    void updateNeighbor(int r, int c);

    void updateGrid(int r, int c, Chess prev);

    [[nodiscard]] Forms calculateScore(int r, int c, Chess chess, Direction dir) const;

    static Forms matchForm(int count, int block, int emptyPos);
};


#endif //GOMOKU_BOARD_H
