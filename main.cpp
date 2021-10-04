#include "MinimaxAI.cpp"
#include "Board.cpp"
#include "jsoncpp/json.h"

int main() {
    Board b();
    MinimaxAI *ai = nullptr;
    Chess identity;
    Json::Reader reader;

    bool first = true;

    while (1) {
        string str;
        getline(cin, str);
        Json::Value input;
        reader.parse(str, input);

        int x, y;
        if (first) {
            x = input["requests"][0]["x"].asInt();
            y = input["requests"][0]["y"].asInt();
            ai = new MinimaxAI(&b, identity = (x < 0 && y < 0 ? black : white), 0, 10);
            first = false;
        } else {
            x = input["x"].asInt();
            y = input["y"].asInt();
        }

        if (x >= 0 && y >= 0) b.set(x, y, static_cast<Chess>(!identity));
        string buff;
        auto res = ai->calculate(&buff);
        b.set(res.x, res.y, identity);

        Json::Value response;
        response["x"] = res.x;
        response["y"] = res.y;

        Json::Value debug;
        debug["_b"] = b.getScore(black);
        debug["_w"] = b.getScore(white);
        debug["msg"] = buff;

        Json::Value ret;
        ret["response"] = response;
        ret["debug"] = debug;
        Json::FastWriter writer;
        cout << writer.write(ret) << endl;
        cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
        fflush(stdout);
    }
}
