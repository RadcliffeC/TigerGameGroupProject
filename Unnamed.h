Point toPoint(const Point_t& pt) {
    return Point(pt.col, pt.row); // Assuming x = col, y = row
}

Point_t toPoint_t(const Point& p) {
    return Point_t{p.y, p.x}; // Assuming row = y, col = x
}



Move_t Move_Unnamed(vector<Token_t> tokens, Color_t turn) {
    Move_t bestMove;
    int bestScore = INT_MAX;
    extern SDL_Plotter g;
    BearGame bear(g);

    if (turn == RED) return bestMove; // Tiger (RED) doesn't auto-move here

    static const Point_t corners[] = {
        {0, 4}, {4, 0}, {4, 8}, {12, 0}, {12, 8}
    };

    auto dist = [](Point_t a, Point_t b) {
        return abs(a.row - b.row) + abs(a.col - b.col);
    };

    auto nearestCorner = [&](Point_t tigerLoc) {
        int minDist = INT_MAX;
        for (auto& c : corners) {
            minDist = min(minDist, dist(tigerLoc, c));
        }
        return minDist;
    };

    auto isSupported = [&](int i, Point_t dst) {
        Point_t orig = tokens[i].location;
        tokens[i].location = dst;
        bool supported = false;

        for (int j = 1; j < tokens.size(); ++j) {
            if (j == i || tokens[j].color != turn) continue;
            if (dist(dst, tokens[j].location) == 1) {
                supported = true;
                break;
            }
        }

        tokens[i].location = orig;
        return supported;
    };

    auto evaluate = [&](vector<Token_t>& simTokens) {
        int steps = 0, jumps = 0;
        Point_t tigerPos = simTokens[0].location;

        for (int r = 0; r < GRID_ROW; ++r) {
            for (int c = 0; c < GRID_COL; ++c) {
                Point_t dst = {r, c};
                int mv = bear.legalMove(toPoint(tigerPos), toPoint(dst));
                if (mv == GOOD_MOVE) steps++;
                if (mv == JUMP_MOVE) jumps++;
            }
        }

        int menLeft = 0;
        for (int i = 1; i < simTokens.size(); ++i) {
            if (simTokens[i].color == turn) menLeft++;
        }

        int cornerDist = nearestCorner(tigerPos);

        return steps + 10 * jumps - 5 * menLeft - 2 * cornerDist;
    };

    vector<Move_t> candidates;

    for (int i = 1; i < tokens.size(); ++i) {
        if (tokens[i].color != turn) continue;
        Point_t src = tokens[i].location;

        for (int r = 0; r < GRID_ROW; ++r) {
            for (int c = 0; c < GRID_COL; ++c) {
                Point_t dst = {r, c};
                if (bear.legalMove(toPoint(src), toPoint(dst)) != GOOD_MOVE) continue;
                if (!isSupported(i, dst)) continue;

                auto backup = tokens[i].location;
                tokens[i].location = dst;
                vector<Token_t> simTokens = tokens;
                int score = evaluate(simTokens);
                tokens[i].location = backup;

                if (score < bestScore) {
                    bestScore = score;
                    candidates.clear();
                    candidates.push_back({tokens[i], dst});
                } else if (score == bestScore) {
                    candidates.push_back({tokens[i], dst});
                }
            }
        }
    }

    if (!candidates.empty()) {
        return candidates[rand() % candidates.size()];
    }

    return {}; // No valid move
}
