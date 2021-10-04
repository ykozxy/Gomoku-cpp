#include "MinimaxAI.h"

#include <iostream>
#include <algorithm>

using namespace std;


Point MinimaxAI::calculate(string *buff) {
    startT = Clock::now();
    m_breakout = false;
    int count = m_board->getCount();

    // First chess
    if (count == 0) {
        srand(time(nullptr));
        int t1 = rand() % 2, t2 = rand() % 2;
        return Point(7 + t1, 7 + t2);
    }

    // Generate points & duplicate
    int size = -1;
    auto points = m_board->heuristicGenerator(m_identity, m_identity, size, false, true);
    assert(size > 0);
    auto *candidates = new Point[size];
    for (int j = 0; j < size; ++j) {
        candidates[j] = Point(points[j]);
        // printf("(%d, %d)", candidates[j].x, candidates[j].y);
    }
    std::cout << std::endl;

    // Do iter
    struct T {
        Point p;
        int depth;
    };
    vector<T> result;

    // int depth = count >= 12 ? 12 : (count >= 6 ? 10 : 8);
    // if (buff != nullptr) *buff = *buff + "d = " + to_string(depth) + "; ";

    for (int i = 2; i <= MINIMAX_DEPTH; i += 2) {
        // printf("Depth (%d/%d)\n", i, depth);
        int resSize = miniMaxWrapper(i, candidates, size);
        assert(resSize > 0);

        if (!m_breakout) {
            for (int j = 0; j < resSize; ++j) {
                auto p = candidates[j];
                T t;
                t.depth = i;
                t.p = Point(p);
                result.push_back(t);
                if (p.ai_score >= _5)
                    break;
            }
        } else break;
    }

    sort(result.begin(), result.end(), [this](const T &a, const T &b) {
        auto compEq = [this](int a, int b) {
            if (abs(a - b) <= m_pruneLimit) return true;
            else return b < a;
        };

        if (abs(a.p.ai_score - b.p.ai_score) > m_pruneLimit)
            return compEq(a.p.ai_score, b.p.ai_score);
        if (a.p.ai_score > 0) {
            // Is winning -> choose lower depth
            if (a.depth == b.depth) return compEq(a.p.ai_score, b.p.ai_score);
            else return b.depth > a.depth;
        } else {
            // Is losing -> choose longer path
            if (a.depth == b.depth) return compEq(a.p.ai_score, b.p.ai_score);
            else return b.depth < a.depth;
        }
    });

    if (buff != nullptr) {
        *buff = *buff + "t: " + to_string(MS_DIFF(startT, Clock::now())) + "; ";
        *buff = *buff + "timeout: " + to_string(m_breakout) + ";";
        *buff = *buff + "final_d: " + to_string(result.at(0).depth) + "; ";
    } else {
        std::cout << "Time: " << MS_DIFF(startT, Clock::now()) << std::endl;
        std::cout << "Final Depth: " << result.at(0).depth << std::endl;
    }

    delete[] candidates;
    return result.at(0).p;
}

int MinimaxAI::miniMaxWrapper(int depth, Point *candidates, int n) {
    // Calculate
    // printf("%d", n);

    for (auto p = candidates; p != candidates + n; p++) {
        IN_RANGE(p->x, p->y);
        m_board->set(p->x, p->y, m_identity);
        int score = miniMaxSearch(depth - 1, numeric_limits<int>::min(), numeric_limits<int>::max(),
                                  static_cast<Chess>(!m_identity), false);
        m_board->set(p->x, p->y, c_empty);
        p->ai_score = score;

        // Check if we still have time
        auto t2 = Clock::now();
        auto totalTime = MS_DIFF(startT, t2);
        if (totalTime >= TIME_LIMIT - 15) {
            // printf("Out of time! [left=%lld]\n", 1000 - totalTime);
            m_breakout = true;
            break;
        }
    }

    sort(candidates, candidates + n, [this](const auto a, const auto b) {
        if (abs(b.ai_score - a.ai_score) < m_pruneLimit)
            return false;
        return b.ai_score < a.ai_score;
    });

    int maxVal = candidates[0].ai_score;
    int j;
    for (j = 0; j < n; ++j) if (maxVal - candidates[j].ai_score > 10) break;

    return j;
}

int MinimaxAI::miniMaxSearch(int depth, int alpha, int beta, Chess player, bool checkmateOnly) {
    assert(player != c_empty);

    // Try use cache
    if (!checkmateOnly) {
        auto cache = m_board->getCache();
        if (cache != nullptr && cache->minimaxNode->depth >= depth)
            return cache->minimaxNode->score;
    }

    // Reach the target depth
    if (depth == 0 && !m_board->hasEnd()) {
        if (!checkmateOnly) {
            // Calculate checkmate for extra layers
            int res = miniMaxSearch(CHECKMATE_DEPTH, alpha, beta, player, true);
            m_board->cache(res, depth);
            return res;
        } else {
            // Checkmate calculation finished, return
            return static_cast<int>(m_board->getScore(m_identity) -
                                    (1. - m_weight) * (float) m_board->getScore(static_cast<Chess>(!m_identity)));
        }
    }

    // Game has ended, return
    if (m_board->hasEnd()) {
        int res = static_cast<int>(m_board->getScore(m_identity) -
                                   (1. - m_weight) * (float) m_board->getScore(static_cast<Chess>(!m_identity)));
        if (!checkmateOnly)
            m_board->cache(res, depth);
        return res;
    }

    // Generate point candidates
    int size = -1;
    auto points = m_board->heuristicGenerator(player, m_identity, size, checkmateOnly, true);
    // printf("%d ", size);

    // If in checkmate mode and no res, end
    if (size <= 0 && checkmateOnly) {
        return static_cast<int>(m_board->getScore(m_identity) -
                                (1. - m_weight) * (float) m_board->getScore(static_cast<Chess>(!m_identity)));
    }
    assert(size > 0);

    // Duplicate points to local var
    auto *points_duplicated = new Point[size];
    for (int j = 0; j < size; ++j) points_duplicated[j] = Point(points[j]);

    if (player == m_identity) {
        // Maximize
        int maxScore = numeric_limits<int>::min();
        for (int j = 0; j < size; ++j) {
            auto p = points_duplicated[j];

            IN_RANGE(p.x, p.y);
            m_board->set(p.x, p.y, player);
            int r = miniMaxSearch(depth - 1, alpha, beta, static_cast<Chess>(!player), checkmateOnly);
            m_board->set(p.x, p.y, c_empty);

            if (MS_DIFF(startT, Clock::now()) >= TIME_LIMIT - 15) {
                m_breakout = true;
                // printf("BREAK: t=%lld, d=%d, %s\n", MS_DIFF(startT, Clock::now()), depth, "MAX");
                break;
            }

            if (r == numeric_limits<int>::max())
                // In case if we reach time limit
                continue;

            int score = r;
            maxScore = max(maxScore, score);

            // Pruning
            alpha = max(alpha, maxScore);
            if (alpha >= beta + m_pruneLimit)
                break;
            if (alpha >= _5)
                break;
        }
        if (!checkmateOnly)
            m_board->cache(maxScore, depth);
        delete[] points_duplicated;
        return maxScore;
    } else {
        // Minimize
        int minScore = numeric_limits<int>::max();
        for (int j = 0; j < size; ++j) {
            auto p = points_duplicated[j];

            IN_RANGE(p.x, p.y);
            m_board->set(p.x, p.y, player);
            int r = miniMaxSearch(depth - 1, alpha, beta, static_cast<Chess>(!player), checkmateOnly);
            m_board->set(p.x, p.y, c_empty);

            if (MS_DIFF(startT, Clock::now()) >= TIME_LIMIT - 15) {
                m_breakout = true;
                // printf("BREAK: t=%lld, d=%d, %s\n", MS_DIFF(startT, Clock::now()), depth, "MIN");
                break;
            }

            if (r == numeric_limits<int>::min())
                // In case if we reach time limit
                continue;

            int score = static_cast<int>(r * (1. + depth / 10.));
            minScore = min(minScore, score);

            // Pruning
            beta = min(beta, minScore);
            if (alpha >= beta + m_pruneLimit)
                break;
            if (beta <= -_5)
                break;
        }
        if (!checkmateOnly)
            m_board->cache(minScore, depth);
        delete[] points_duplicated;
        return minScore;
    }
}

