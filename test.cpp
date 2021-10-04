#include "Board.h"
#include "MinimaxAI.h"

#include <iostream>

using namespace std;

int main() {
    auto *b = new Board(), *b1 = new Board(), *b2 = new Board();
    MinimaxAI ai_b(b1, black, 0, 10);
    MinimaxAI ai_w(b1, white, 0, 10);
    // MctAI ai_w(b2, white);

    string t;
    while (!b->hasEnd()) {
        std::cout << "\n========================================" << std::endl;
        auto p = ai_b.calculate();
        b->set(p.x, p.y, black);
        b1->set(p.x, p.y, black);
        b2->set(p.x, p.y, black);
        std::cout << b->to_string();
        printf("(%d, %d), score=%d\n", p.x, p.y, p.ai_score);
        printf("Current: black = %d, white = %d\n", b->getScore(black), b->getScore(white));
        if (b->hasEnd())
            break;
        // cin >> t;

        std::cout << "\n========================================" << std::endl;
        auto q = ai_w.calculate();
        b->set(q.x, q.y, white);
        b1->set(q.x, q.y, white);
        b2->set(q.x, q.y, white);
        std::cout << b->to_string();
        // printf("(%d, %d), score=%d\n", q.x, q.y, q.ai_score);
        printf("Current: black = %d, white = %d\n", b->getScore(black), b->getScore(white));
        // cin >> t;
    }
    std::cout << b1->getCachedSize() << std::endl;
}
