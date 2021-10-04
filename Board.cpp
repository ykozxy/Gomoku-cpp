#include "Board.h"

#include <algorithm>

using namespace std;

Board::Board() {
    // Fill board
    for (auto &i : m_board) for (auto &j : i) j = c_empty;

    // Init zobrist
    srand(time(nullptr));
    m_zobristCode = (static_cast<long>(rand()) << (sizeof(int) * 8)) | rand();
    for (auto &i : m_zobristTable)
        for (auto &j : i)
            for (long &k : j)
                k = (static_cast<long>(rand()) << (sizeof(int) * 8)) | rand();
}

void Board::set(int r, int c, Chess player) {
    IN_RANGE(r, c);

    // Ensure the input chess is valid
    Chess prev = m_board[r][c];
    assert(!(prev != c_empty && player != c_empty));

    // Adjust chess counter
    if (prev == c_empty) {
        if (player != c_empty) m_numChess++;
    } else if (player == c_empty) m_numChess--;

    // Set chess and do update
    m_board[r][c] = player;
    m_zobristCode ^= m_zobristTable[player == c_empty ? prev : player][r][c];
    updateGrid(r, c, prev);
    updateNeighbor(r, c);
}

int Board::getScore(Chess player) const {
    return m_totalScore[player];
}

int Board::getScore(int r, int c, Chess player) const {
    int res = 0;
    for (const auto &s : m_pointScores[player]) res += s[r][c];
    return res;
}

int Board::getCount() const {
    return m_numChess;
}

Chess Board::getGrid(int r, int c) const {
    return m_board[r][c];
}

bool Board::hasEnd() const {
    return m_win;
}

bool Board::hasNeighbor(int r, int c, int range, int count) const {
    assert(range == 2 || range == 1);
    int dist_1 = m_neighborCount[0][r][c], dist_2 = m_neighborCount[1][r][c];
    assert(dist_1 >= 0 && dist_2 >= 0);
    if (range == 1)
        return dist_1 >= count;
    else
        return dist_2 > 0 || dist_1 >= count;
}

void Board::cache(int score, int depth) {
    m_cache.insert({m_zobristCode, new CacheData(score, depth)});
}

CacheData *Board::getCache() const {
    if (m_cache.find(m_zobristCode) == m_cache.end())
        return nullptr;
    return m_cache.at(m_zobristCode);
}

unsigned long Board::getCachedSize() const {
    return m_cache.size() * sizeof(CacheData);
}

std::string Board::to_string(std::vector<Point *> *planned) {
    std::string res = "  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4\n";
    for (int i = 0; i < BOARD_SIZE; ++i) {
        res += std::to_string(i % 10) + " ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            bool flag = false;
            if (planned != nullptr)
                for (const auto &p : *planned) {
                    if (p->x == i && p->y == j) {
                        flag = true;
                        break;
                    }
                }
            auto ele = m_board[i][j];
            switch (ele) {
                case c_empty:
                    if (flag) res += "x";
                    else res += ".";
                    break;
                case white:
                    res += "○";
                    break;
                case black:
                    res += "●";
                    break;
            }
            res += " ";
        }
        res += "\n";
    }
    return res;
}

Point *Board::heuristicGenerator(Chess player, Chess ai_id, int &resSize, bool checkmateOnly, bool do_sort) {
    assert(getCount() > 0);
    auto oppo = static_cast<Chess>(!player);

    int i_ai_5 = 0, i_op_5 = 0, i_ai_4p = 0, i_op_4p = 0, i_ai_4m = 0, i_op_4m = 0, i_ai_combo = 0, i_op_combo = 0,
            i_ai_double3 = 0, i_op_double3 = 0, i_ai_3p = 0, i_op_3p = 0, i_ai_2p = 0, i_op_2p = 0, i_neighbor = 0;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (getGrid(r, c) != c_empty) continue;
            if (!hasNeighbor(r, c, m_numChess < 6 ? 1 : 2, m_numChess < 6 ? 1 : 2)) continue;

            Forms ai_score[4], op_score[4];
            int ai_total, op_total;
            ai_total = (ai_score[0] = m_pointScores[player][0][r][c]) +
                       (ai_score[1] = m_pointScores[player][1][r][c]) +
                       (ai_score[2] = m_pointScores[player][2][r][c]) +
                       (ai_score[3] = m_pointScores[player][3][r][c]);
            op_total = (op_score[0] = m_pointScores[oppo][0][r][c]) +
                       (op_score[1] = m_pointScores[oppo][1][r][c]) +
                       (op_score[2] = m_pointScores[oppo][2][r][c]) +
                       (op_score[3] = m_pointScores[oppo][3][r][c]);


            Point p(r, c, ai_total, op_total);
            int ai_4m_count = 0, op_4m_count = 0, ai_3p_count = 0, op_3p_count = 0, ai_3m_count = 0, op_3m_count = 0,
                    ai_2p_count = 0, op_2p_count = 0;

            auto ai = ai_score[0], op = op_score[0];
            // Checkmate!
            if (ai == _5) {
                ai_5[i_ai_5++] = p;
                break;
            } else if (op == _5) {
                op_5[i_op_5++] = p;
                break;
            } else if (ai == _4p) {
                ai_4p[i_ai_4p++] = p;
                break;
            } else if (op == _4p) {
                op_4p[i_op_4p++] = p;
                break;
            }
            switch (ai) {
                case _4m:
                    ai_4m_count++;
                    break;
                case _3p:
                    ai_3p_count++;
                    break;
                case _3m:
                    ai_3m_count++;
                    break;
                case _2p:
                    ai_2p_count++;
                default:
                    break;
            }
            switch (op) {
                case _4m:
                    op_4m_count++;
                    break;
                case _3p:
                    op_3p_count++;
                    break;
                case _3m:
                    op_3m_count++;
                    break;
                case _2p:
                    op_2p_count++;
                default:
                    break;
            }

            ai = ai_score[1], op = op_score[1];
            // Checkmate!
            if (ai == _5) {
                ai_5[i_ai_5++] = p;
                break;
            } else if (op == _5) {
                op_5[i_op_5++] = p;
                break;
            } else if (ai == _4p) {
                ai_4p[i_ai_4p++] = p;
                break;
            } else if (op == _4p) {
                op_4p[i_op_4p++] = p;
                break;
            }
            switch (ai) {
                case _4m:
                    ai_4m_count++;
                    break;
                case _3p:
                    ai_3p_count++;
                    break;
                case _3m:
                    ai_3m_count++;
                    break;
                case _2p:
                    ai_2p_count++;
                default:
                    break;
            }
            switch (op) {
                case _4m:
                    op_4m_count++;
                    break;
                case _3p:
                    op_3p_count++;
                    break;
                case _3m:
                    op_3m_count++;
                    break;
                case _2p:
                    op_2p_count++;
                default:
                    break;
            }

            ai = ai_score[2], op = op_score[2];
            // Checkmate!
            if (ai == _5) {
                ai_5[i_ai_5++] = p;
                break;
            } else if (op == _5) {
                op_5[i_op_5++] = p;
                break;
            } else if (ai == _4p) {
                ai_4p[i_ai_4p++] = p;
                break;
            } else if (op == _4p) {
                op_4p[i_op_4p++] = p;
                break;
            }
            switch (ai) {
                case _4m:
                    ai_4m_count++;
                    break;
                case _3p:
                    ai_3p_count++;
                    break;
                case _3m:
                    ai_3m_count++;
                    break;
                case _2p:
                    ai_2p_count++;
                default:
                    break;
            }
            switch (op) {
                case _4m:
                    op_4m_count++;
                    break;
                case _3p:
                    op_3p_count++;
                    break;
                case _3m:
                    op_3m_count++;
                    break;
                case _2p:
                    op_2p_count++;
                default:
                    break;
            }

            ai = ai_score[3], op = op_score[3];
            // Checkmate!
            if (ai == _5) {
                ai_5[i_ai_5++] = p;
                break;
            } else if (op == _5) {
                op_5[i_op_5++] = p;
                break;
            } else if (ai == _4p) {
                ai_4p[i_ai_4p++] = p;
                break;
            } else if (op == _4p) {
                op_4p[i_op_4p++] = p;
                break;
            }
            switch (ai) {
                case _4m:
                    ai_4m_count++;
                    break;
                case _3p:
                    ai_3p_count++;
                    break;
                case _3m:
                    ai_3m_count++;
                    break;
                case _2p:
                    ai_2p_count++;
                default:
                    break;
            }
            switch (op) {
                case _4m:
                    op_4m_count++;
                    break;
                case _3p:
                    op_3p_count++;
                    break;
                case _3m:
                    op_3m_count++;
                    break;
                case _2p:
                    op_2p_count++;
                default:
                    break;
            }

            // TODO: improve score tolerance
            if (ai_3p_count >= 2)
                ai_double3[i_ai_double3++] = p;
            else if (op_3p_count >= 2)
                op_double3[i_op_double3++] = p;
            else if (ai_3p_count + ai_4m_count >= 2)
                ai_combo[i_ai_combo++] = p;
            else if (op_3p_count + op_4m_count >= 2)
                op_combo[i_op_combo++] = p;
            else if (ai_4m_count)
                ai_4m[i_ai_4m++] = p;
            else if (op_4m_count)
                op_4m[i_op_4m++] = p;
            else if (ai_3p_count)
                ai_3p[i_ai_3p++] = p;
            else if (op_3p_count)
                op_3p[i_op_3p++] = p;
            else if (ai_2p_count)
                ai_2p[i_ai_2p++] = p;
            else if (op_2p_count)
                op_2p[i_op_2p++] = p;
            else neighbor[i_neighbor++] = p;
        }
    }

    auto aiCom = [](const Point &a, const Point &b) {
        return b.ai_score < a.ai_score;
    };
    auto opCom = [](const Point &a, const Point &b) {
        return b.op_score < a.op_score;
    };
    auto bothCom = [](const Point &a, const Point &b) {
        return max(b.op_score, b.ai_score) < max(a.op_score, a.ai_score);
    };
    auto concat = [](Point *a, int &n1, Point *b, int n2) {
        assert(n1 + n2 <= BOARD_SIZE * BOARD_SIZE / 2);
        for (int i = 0; i < n2; ++i)
            a[n1++] = b[i];
    };

    // Check 5
    if (i_ai_5 != 0 || i_op_5 != 0) {
        concat(ai_5, i_ai_5, op_5, i_op_5);
        resSize = i_ai_5;
        return ai_5;
    }

    // Check 4
    if ((resSize = i_ai_4p) != 0)
        return ai_4p;
    if ((resSize = i_op_4p) != 0)
        return op_4p;

    // Combo
    if ((resSize = i_ai_combo) != 0)
        return ai_combo;
    if ((resSize = i_op_combo) != 0)
        return op_combo;

    // Double 3
    if ((resSize = i_ai_double3) != 0)
        return ai_double3;
    if ((resSize = i_op_double3) != 0)
        return op_double3;

    if (checkmateOnly) {
        if (player == ai_id) {
            // Only search 3 & 4m
            concat(ai_3p, i_ai_3p, ai_4m, i_ai_4m);
            resSize = i_ai_3p;
            return ai_3p;
        } else {
            // Search 3 & 4m for both side
            concat(ai_3p, i_ai_3p, ai_4m, i_ai_4m);
            concat(ai_3p, i_ai_3p, op_4m, i_op_4m);
            concat(ai_3p, i_ai_3p, op_3p, i_op_3p);
            resSize = i_ai_3p;
            return ai_3p;
        }
    }
    assert(!checkmateOnly);

    // Oppo 3+
    if (i_op_3p && !i_op_4m) {
        concat(op_3p, i_op_3p, ai_4m, i_ai_4m);
        concat(op_3p, i_op_3p, ai_3p, i_ai_3p);
        resSize = i_op_3p;
        return op_3p;
    }

    // 3 & 4
    concat(ai_3p, i_ai_3p, op_3p, i_op_3p);
    concat(ai_3p, i_ai_3p, ai_4m, i_ai_4m);
    concat(ai_3p, i_ai_3p, op_4m, i_op_4m);
    if (i_ai_3p != 0) {
        resSize = i_ai_3p;
        return ai_3p;
    }


    concat(ai_2p, i_ai_2p, op_2p, i_op_2p);
    sort(ai_2p, ai_2p + i_ai_2p, bothCom);
    if ((resSize = i_ai_2p) != 0) {
        if (resSize > 20) resSize = 20;
        return ai_2p;
    }

    // Others
    sort(neighbor, neighbor + i_neighbor, bothCom);
    resSize = i_neighbor;
    if (resSize > 20) resSize = 20;
    return neighbor;
}


void Board::updateNeighbor(int r, int c) {
    // Update the 2x2 range
    int adder = m_board[r][c] == c_empty ? -1 : 1;
    for (int i = max(0, r - 2); i <= min(BOARD_SIZE - 1, r + 2); i++) {
        for (int j = max(0, c - 2); j <= min(BOARD_SIZE - 1, c + 2); j++) {
            if (abs(i - r) <= 1 && abs(j - c) <= 1)
                // Dist = 1 || 0
                m_neighborCount[0][i][j] += adder;
            else
                // Dist = 2
                m_neighborCount[1][i][j] += adder;
            assert(m_neighborCount[0][i][j] >= 0);
            assert(m_neighborCount[1][i][j] >= 0);
        }
    }
}

void Board::updateGrid(int r, int c, Chess prev) {
    const auto chess = m_board[r][c];

    if (chess != c_empty) {
        // empty -> chess: Calculate score @ (x, y)
        m_totalScore[chess] +=
                (m_pointScores[chess][horizontal][r][c] = calculateScore(r, c, chess, horizontal)) +
                (m_pointScores[chess][vertical][r][c] = calculateScore(r, c, chess, vertical)) +
                (m_pointScores[chess][diag_RU][r][c] = calculateScore(r, c, chess, diag_RU)) +
                (m_pointScores[chess][diag_LU][r][c] = calculateScore(r, c, chess, diag_LU));
    } else {
        // chess -> empty: Revert score @ (x, y)
        m_totalScore[prev] -= getScore(r, c, prev);
    }

    // Update horizontally
    bool flag[2]{};
    int count[2]{};
    for (int ct = c - 1; ct >= max(0, c - SCORE_RANGE); ct--) {
        IN_RANGE(r, ct);
        auto ele = m_board[r][ct];
        if (ele == c_empty) {

            m_pointScores[black][horizontal][r][ct] = calculateScore(r, ct, black, horizontal);
            m_pointScores[white][horizontal][r][ct] = calculateScore(r, ct, white, horizontal);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][horizontal][r][ct];
            m_totalScore[ele] += (m_pointScores[ele][horizontal][r][ct] = calculateScore(r, ct, ele, horizontal));

        }
    }
    flag[0] = flag[1] = false;
    for (int ct = c + 1; ct < min(BOARD_SIZE, c + SCORE_RANGE); ct++) {
        IN_RANGE(r, ct);
        auto ele = m_board[r][ct];
        if (ele == c_empty) {

            m_pointScores[black][horizontal][r][ct] = calculateScore(r, ct, black, horizontal);
            m_pointScores[white][horizontal][r][ct] = calculateScore(r, ct, white, horizontal);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][horizontal][r][ct];
            m_totalScore[ele] += (m_pointScores[ele][horizontal][r][ct] = calculateScore(r, ct, ele, horizontal));

        }
    }
    if (chess == c_empty) {
        if (m_win && count[prev] < 4)
            m_win = false;
    } else if (count[chess] >= 4) {
        m_win = true;
    }

    // Update vertically
    count[0] = count[1] = flag[0] = flag[1] = false;
    for (int rt = r - 1; rt >= max(0, r - SCORE_RANGE); rt--) {
        IN_RANGE(rt, c);
        auto ele = m_board[rt][c];
        if (ele == c_empty) {

            m_pointScores[black][vertical][rt][c] = calculateScore(rt, c, black, vertical);
            m_pointScores[white][vertical][rt][c] = calculateScore(rt, c, white, vertical);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][vertical][rt][c];
            m_totalScore[ele] += (m_pointScores[ele][vertical][rt][c] = calculateScore(rt, c, ele, vertical));

        }
    }
    flag[0] = flag[1] = false;
    for (int rt = r + 1; rt < min(BOARD_SIZE, r + SCORE_RANGE); rt++) {
        IN_RANGE(rt, c);
        auto ele = m_board[rt][c];
        if (ele == c_empty) {

            m_pointScores[black][vertical][rt][c] = calculateScore(rt, c, black, vertical);
            m_pointScores[white][vertical][rt][c] = calculateScore(rt, c, white, vertical);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][vertical][rt][c];
            m_totalScore[ele] += (m_pointScores[ele][vertical][rt][c] = calculateScore(rt, c, ele, vertical));

        }
    }
    if (chess == c_empty) {
        if (m_win && count[prev] < 4)
            m_win = false;
    } else if (count[chess] >= 4) {
        m_win = true;
    }

    // Update diagonally (LU -> RD)
    count[0] = count[1] = flag[0] = flag[1] = false;
    for (int t = 1;
         (c - t >= max(0, c - SCORE_RANGE)) && (r - t >= max(0, r - SCORE_RANGE));
         t++) {
        int rt = r - t, ct = c - t;
        IN_RANGE(rt, ct);
        auto ele = m_board[rt][ct];
        if (ele == c_empty) {

            m_pointScores[black][diag_LU][rt][ct] = calculateScore(rt, ct, black, diag_LU);
            m_pointScores[white][diag_LU][rt][ct] = calculateScore(rt, ct, white, diag_LU);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][diag_LU][rt][ct];
            m_totalScore[ele] += (m_pointScores[ele][diag_LU][rt][ct] = calculateScore(rt, ct, ele, diag_LU));

        }
    }
    flag[0] = flag[1] = false;
    for (int t = 1;
         (c + t < min(BOARD_SIZE, c + SCORE_RANGE)) && (r + t < min(BOARD_SIZE, r + SCORE_RANGE));
         t++) {
        int rt = r + t, ct = c + t;
        IN_RANGE(rt, ct);
        auto ele = m_board[rt][ct];
        if (ele == c_empty) {

            m_pointScores[black][diag_LU][rt][ct] = calculateScore(rt, ct, black, diag_LU);
            m_pointScores[white][diag_LU][rt][ct] = calculateScore(rt, ct, white, diag_LU);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][diag_LU][rt][ct];
            m_totalScore[ele] += (m_pointScores[ele][diag_LU][rt][ct] = calculateScore(rt, ct, ele, diag_LU));

        }
    }
    if (chess == c_empty) {
        if (m_win && count[prev] < 4)
            m_win = false;
    } else if (count[chess] >= 4) {
        m_win = true;
    }

    // Update diagonally (RU -> LD)
    count[0] = count[1] = flag[0] = flag[1] = false;
    for (int t = 1;
         (c + t < min(BOARD_SIZE, c + SCORE_RANGE)) && (r - t >= max(0, r - SCORE_RANGE));
         t++) {
        int rt = r - t, ct = c + t;
        IN_RANGE(rt, ct);
        auto ele = m_board[rt][ct];
        if (ele == c_empty) {

            m_pointScores[black][diag_RU][rt][ct] = calculateScore(rt, ct, black, diag_RU);
            m_pointScores[white][diag_RU][rt][ct] = calculateScore(rt, ct, white, diag_RU);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][diag_RU][rt][ct];
            m_totalScore[ele] += (m_pointScores[ele][diag_RU][rt][ct] = calculateScore(rt, ct, ele, diag_RU));

        }
    }
    flag[0] = flag[1] = false;
    for (int t = 1;
         (c - t >= max(0, c - SCORE_RANGE)) && (r + t < min(BOARD_SIZE, r + SCORE_RANGE));
         t++) {
        int rt = r + t, ct = c - t;
        IN_RANGE(rt, ct);
        auto ele = m_board[rt][ct];
        if (ele == c_empty) {

            m_pointScores[black][diag_RU][rt][ct] = calculateScore(rt, ct, black, diag_RU);
            m_pointScores[white][diag_RU][rt][ct] = calculateScore(rt, ct, white, diag_RU);

        } else {
            for (int i = 0; i < 2; ++i) {
                if (!flag[i] && ele == i)
                    count[i]++;
                else
                    flag[i] = true;
            }

            m_totalScore[ele] -= m_pointScores[ele][diag_RU][rt][ct];
            m_totalScore[ele] += (m_pointScores[ele][diag_RU][rt][ct] = calculateScore(rt, ct, ele, diag_RU));

        }
    }
    if (chess == c_empty) {
        if (m_win && count[prev] < 4)
            m_win = false;
    } else if (count[chess] >= 4) {
        m_win = true;
    }
}

Forms Board::calculateScore(int r, int c, Chess chess, Direction dir) const {
    int count = 1;
    int block = 0;
    int emptyPos = -1;

    switch (dir) {
        case horizontal: {
            // Left
            for (int ct = c - 1; ct >= c - SCORE_RANGE; ct--) {
                if (ct < 0) {
                    block++;
                    break;
                }
                IN_RANGE(r, ct);
                auto ele = m_board[r][ct];
                if (ele == chess) {
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && ct > 0 && m_board[r][ct - 1] == chess)
                        emptyPos = count;
                    else break;
                } else {
                    block++;
                    break;
                }
            }

            // Right
            for (int ct = c + 1; ct <= c + SCORE_RANGE; ct++) {
                if (ct >= BOARD_SIZE) {
                    block++;
                    break;
                }
                IN_RANGE(r, ct);
                auto ele = m_board[r][ct];
                if (ele == chess) {
                    if (emptyPos != -1) emptyPos++;
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && ct < BOARD_SIZE - 1 && m_board[r][ct + 1] == chess)
                        emptyPos = 0;
                    else break;
                } else {
                    block++;
                    break;
                }
            }
            break;
        }
        case vertical: {
            // Up
            for (int rt = r - 1; rt >= r - SCORE_RANGE; rt--) {
                if (rt < 0) {
                    block++;
                    break;
                }
                IN_RANGE(rt, c);
                auto ele = m_board[rt][c];
                if (ele == chess) {
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && rt > 0 && m_board[rt - 1][c] == chess)
                        emptyPos = count;
                    else break;
                } else {
                    block++;
                    break;
                }
            }

            // Down
            for (int rt = r + 1; rt <= r + SCORE_RANGE; rt++) {
                if (rt >= BOARD_SIZE) {
                    block++;
                    break;
                }
                IN_RANGE(rt, c);
                auto ele = m_board[rt][c];
                if (ele == chess) {
                    if (emptyPos != -1) emptyPos++;
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && rt < BOARD_SIZE - 1 && m_board[rt + 1][c] == chess)
                        emptyPos = 0;
                    else break;
                } else {
                    block++;
                    break;
                }
            }
            break;
        }
        case diag_LU: {
            // LU
            for (int t = 1; t <= SCORE_RANGE; t++) {
                int rt = r - t, ct = c - t;
                if (rt < 0 || ct < 0) {
                    block++;
                    break;
                }

                IN_RANGE(rt, ct);
                auto ele = m_board[rt][ct];

                if (ele == chess) {
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && (ct > 0 && rt > 0) && m_board[rt - 1][ct - 1] == chess)
                        emptyPos = count;
                    else break;
                } else {
                    block++;
                    break;
                }
            }

            // RD
            for (int t = 1; t <= SCORE_RANGE; t++) {
                int rt = r + t, ct = c + t;
                if (rt >= BOARD_SIZE || ct >= BOARD_SIZE) {
                    block++;
                    break;
                }

                IN_RANGE(rt, ct);
                auto ele = m_board[rt][ct];
                if (ele == chess) {
                    if (emptyPos != -1) emptyPos++;
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && (ct + 1 < BOARD_SIZE && rt + 1 < BOARD_SIZE) &&
                        m_board[rt + 1][ct + 1] == chess)
                        emptyPos = 0;
                    else break;
                } else {
                    block++;
                    break;
                }
            }
            break;
        }
        case diag_RU: {
            // RU
            for (int t = 1; t <= SCORE_RANGE; t++) {
                int rt = r - t, ct = c + t;
                if (rt < 0 || ct >= BOARD_SIZE) {
                    block++;
                    break;
                }

                IN_RANGE(rt, ct);
                auto ele = m_board[rt][ct];

                if (ele == chess) {
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && (ct + 1 < BOARD_SIZE && rt > 0) &&
                        m_board[rt - 1][ct + 1] == chess)
                        emptyPos = count;
                    else break;
                } else {
                    block++;
                    break;
                }
            }

            // LD
            for (int t = 1; t <= SCORE_RANGE; t++) {
                int rt = r + t, ct = c - t;
                if (ct < 0 || rt >= BOARD_SIZE) {
                    block++;
                    break;
                }

                IN_RANGE(rt, ct);
                auto ele = m_board[rt][ct];
                if (ele == chess) {
                    if (emptyPos != -1) emptyPos++;
                    count++;
                } else if (ele == c_empty) {
                    if (emptyPos == -1 && (ct > 0 && rt + 1 < BOARD_SIZE) &&
                        m_board[rt + 1][ct - 1] == chess)
                        emptyPos = 0;
                    else break;
                } else {
                    block++;
                    break;
                }
            }
            break;
        }
    }

    return matchForm(count, block, emptyPos);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-multiway-paths-covered"

Forms Board::matchForm(int count, int block, int emptyPos) {
    if (emptyPos <= 0) {
        if (count >= 5)
            return _5;
        if (block == 0) {
            switch (count) {
                case 1:
                    return _1p;
                case 2:
                    return _2p;
                case 3:
                    return _3p;
                case 4:
                    return _4p;
            }
        } else if (block == 1) {
            switch (count) {
                case 1:
                    return _1m;
                case 2:
                    return _2m;
                case 3:
                    return _3m;
                case 4:
                    return _4m;
            }
        }
    } else if (emptyPos == 1 || emptyPos == count - 1) {
        // Empty on the first position
        if (count >= 6)
            return _5;
        if (block == 0) {
            switch (count) {
                case 2:
                    // return _2p / 2;
                    return _2p_spaced;
                case 3:
                    return _3p;
                case 4:
                    return _4m;
                case 5:
                    return _4p;
            }
        } else if (block == 1) {
            switch (count) {
                case 2:
                    return _2m;
                case 3:
                    return _3m;
                case 4:
                case 5:
                    return _4m;
            }
        }

    } else if (emptyPos == 2 || emptyPos == count - 2) {
        // Empty on the second position
        if (count >= 7)
            return _5;
        if (block == 0) {
            switch (count) {
                case 3:
                    return _3p;
                case 4:
                case 5:
                    return _4m;
                case 6:
                    return _4p;
            }
        } else if (block == 1) {
            switch (count) {
                case 3:
                    return _3m;
                case 4:
                case 5:
                    return _4m;
                case 6:
                    return _4p;
            }
        } else if (block == 2) {
            switch (count) {
                case 4:
                case 5:
                case 6:
                    return _4m;
            }
        }

    } else if (emptyPos == 3 || emptyPos == count - 3) {
        // Empty on the third position
        if (count >= 8)
            return _5;
        if (block == 0) {
            switch (count) {
                case 4:
                case 5:
                    return _3p;
                case 6:
                    return _4m;
                case 7:
                    return _4p;
            }
        } else if (block == 1) {
            switch (count) {
                case 4:
                case 5:
                case 6:
                    return _4m;
                case 7:
                    return _4p;
            }
        } else if (block == 2) {
            switch (count) {
                case 4:
                case 5:
                case 6:
                case 7:
                    return _4m;
            }
        }
    } else if (emptyPos == 4 || emptyPos == count - 4) {
        // Empty on the fourth position
        if (count > 9)
            return _5;
        if (block == 0) {
            switch (count) {
                case 5:
                case 6:
                case 7:
                case 8:
                    return _4p;
            }
        } else if (block == 1) {
            switch (count) {
                case 4:
                case 5:
                case 6:
                case 7:
                    return _4m;
                case 8:
                    return _4p;
            }
        } else if (block == 2) {
            switch (count) {
                case 5:
                case 6:
                case 7:
                case 8:
                    return _4m;
            }
        }
    } else if (emptyPos == 5 || emptyPos == count - 5)
        return _5;
    return _empty;
}

#pragma clang diagnostic pop
