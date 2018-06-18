#include "AI.h"
#include "Board.h"
#include "Util.h"
#include "MidEvaluator.h"
#include <iomanip>

using namespace std;

RandomAI::RandomAI() : AI() {

}

std::uint64_t RandomAI::act(Board& board) {
    uint64_t movables = board.getMovablePos();
    uint64_t hand = 0;

    if (movables == 0) {
        return 0;
    }

    if (is_debug)
        cout << getClassName();

    int idx = Util::random(Util::bitCountInt8(movables)) + 1;
    for (int i = 0; i < idx; i++) {
        hand = movables & (-movables);
        movables ^= hand;
    }

    if (is_debug)
        cout << ", move:" << Util::pointStr(hand, BOARD_SIZE) << endl;
    return hand;
}

NegaMaxAI::NegaMaxAI() : AI() {
    presearch_depth = 3;
    this->normal_depth = 9;
    this->wld_depth = 17;
    this->perfect_depth = 15;
}

NegaMaxAI::NegaMaxAI(const int normal_depth) : AI() {
    presearch_depth = 3;
    this->normal_depth = normal_depth;
    this->wld_depth = 17;
    this->perfect_depth = 15;
}

NegaMaxAI::NegaMaxAI(const int normal_depth, const int wld_depth, const int perfect_depth) : AI() {
    presearch_depth = 3;
    this->normal_depth = normal_depth;
    this->wld_depth = wld_depth;
    this->perfect_depth = perfect_depth;
}

NegaMaxAI::~NegaMaxAI() {

}

std::uint64_t NegaMaxAI::act(Board& board) {
    nodes = 0;
    leaves = 0;

    uint64_t movables = board.getMovablePos();

    if (movables == 0)
        return 0;

    const int movables_size = Util::bitCountInt8(movables);
    if (movables_size == 1)
        return movables;

    evaluator = new MidEvaluator();

    int emptyNum = board.countEmpty();
    if (emptyNum <= wld_depth) {
        delete evaluator;
        depth_max = numeric_limits<int>::max();
        if (emptyNum <= perfect_depth)
            evaluator = new PerfectEvaluator();
        else
            evaluator = new WLDEvaluator();
    } else {
        depth_max = normal_depth;
    }

    uint64_t sorted_moves[movables_size];
    sort(board, movables, depth_max - presearch_depth, sorted_moves, movables_size);

    if (is_debug)
        cout << getClassName() << "(" << evaluator->getClassName() << "), turns:" << dec << board.getTurns();

    int eval = MIN_SCORE;
    int alpha = MIN_SCORE;
    int beta = MAX_SCORE;
    uint64_t best = 0;
    uint64_t next_hand = 0;
    for (int i = 0; i < movables_size; i++) {
        next_hand = sorted_moves[i];
        board.move(next_hand);
        eval = -search(board, -beta, -alpha, 0);
        // cout << "depth_max:" << depth_max
        //     << ",move:" << Util::pointStr(next_hand, BOARD_SIZE) << ", eval:" << dec << eval << endl;
        //cout << board.str() << endl;
        board.undo();

        if (eval > alpha) {
            alpha = eval;
            best = next_hand;
        }
    }

    delete evaluator;
    evaluator = NULL;

    if (is_debug) {
        cout << ", move:" << Util::pointStr(best, BOARD_SIZE) << ", eval:" << dec << alpha;
        cout << ", leaves:" << leaves << ", nodes:" << nodes << endl;
    }

    return best;
}

void NegaMaxAI::sort(Board& board, uint64_t movables, int depth, uint64_t sorted_moves[], const int movables_size) {
    vector<Move> moves;

    int eval;
    uint64_t next_hand;
    while (movables != 0) {
        next_hand = movables & (-movables);

        board.move(next_hand);
        eval = -search(board, MIN_SCORE, MAX_SCORE, depth);
        board.undo();

        Move move(next_hand, eval);
        moves.push_back(move);

        movables ^= next_hand;
    }

    stable_sort(moves.begin(), moves.end(), MoveGreater());

    for (int i = 0; i < movables_size; i++) {
        sorted_moves[i] = moves[i].point;
    }

    return;
}

int NegaMaxAI::search(Board& board, int alpha, int beta, int depth) {
    if (depth >= depth_max || board.isFull()) {
        leaves++;
        return evaluator->evaluate(board);
    }

    nodes++;

    int eval;
    uint64_t movables = board.getMovablePos();

    if (movables == 0) {
        if (board.isGameOver()) {
            leaves++;
            return evaluator->evaluate(board);
        } else {
            board.pass();
            eval = -search(board, -beta, -alpha, depth + 1);
            board.undo();
            return eval;
        }
    }

    uint64_t next_hand;
    while (movables != 0) {
        next_hand = movables & (-movables);
        //std::cout << "search act:" << std::setfill('0') << std::setw(16) << hex << next_hand << std::endl;

        board.move(next_hand);
        eval = -search(board, -beta, -alpha, depth + 1);
        // std::cout << "search act:" << std::setfill('0') << std::setw(16) << hex << next_hand
        //     << ", eval:" << dec << eval
        //     << ", alpha:" << alpha << ", beta:" << beta << std::endl;
        board.undo();

        if (eval >= beta)
            return eval;

        if (eval > alpha)
            alpha = eval;

        movables ^= next_hand;
    }

    return alpha;
}

NegaScoutAI::NegaScoutAI() : AI() {
    presearch_depth = 3;
    normal_depth = 9;
    wld_depth = 17;
    perfect_depth = 15;
}

NegaScoutAI::NegaScoutAI(const int normal_depth) : AI() {
    presearch_depth = 3;
    this->normal_depth = normal_depth;
    this->wld_depth = 17;
    this->perfect_depth = 15;
}

NegaScoutAI::NegaScoutAI(const int normal_depth, const int wld_depth, const int perfect_depth) : AI() {
    presearch_depth = 3;
    this->normal_depth = normal_depth;
    this->wld_depth = wld_depth;
    this->perfect_depth = perfect_depth;
}

NegaScoutAI::~NegaScoutAI() {

}

std::uint64_t NegaScoutAI::act(Board& board) {
    nodes = 0;
    leaves = 0;

    uint64_t movables = board.getMovablePos();

    if (movables == 0)
        return 0;

    const int movables_size = Util::bitCountInt8(movables);
    if (movables_size == 1)
        return movables;

    int limit;
    evaluator = new MidEvaluator();
    uint64_t sorted_moves[movables_size];
    sort(board, movables, presearch_depth, sorted_moves, movables_size);

    int emptyNum = board.countEmpty();
    if (emptyNum <= wld_depth) {
        delete evaluator;
        limit = numeric_limits<int>::max();
        if (emptyNum <= perfect_depth)
            evaluator = new PerfectEvaluator();
        else
            evaluator = new WLDEvaluator();
    } else {
        limit = normal_depth;
    }

    if (is_debug)
        cout << getClassName() << "(" << evaluator->getClassName() << "), turns:" << dec << board.getTurns();

    int eval = MIN_SCORE;
    int alpha = MIN_SCORE;
    int beta = MAX_SCORE;
    uint64_t best = 0;
    for (int i = 0; i < movables_size; i++) {
        board.move(sorted_moves[i]);
        eval = -negaScout(board, limit - 1, -beta, -alpha);
        //cout << "move:" << Util::pointStr(sorted_moves[i], BOARD_SIZE) << ", eval:" << eval << endl;
        //cout << board.str() << endl;
        board.undo();

        if (eval > alpha) {
            alpha = eval;
            best = sorted_moves[i];
        }
    }

    delete evaluator;
    evaluator = NULL;

    if (is_debug) {
        cout << ", move:" << Util::pointStr(best, BOARD_SIZE) << ", eval:" << dec << alpha;
        cout << ", leaves:" << leaves << ", nodes" << nodes << endl;
    }

    return best;
}

void NegaScoutAI::sort(Board& board, uint64_t movables, int limit, uint64_t sorted_moves[], const int movables_size) {
    vector<Move> moves;

    int eval;
    uint64_t next_hand;
    while (movables != 0) {
        next_hand = movables & (-movables);

        board.move(next_hand);
        eval = -negaScout(board, limit - 1, MIN_SCORE, MAX_SCORE);
        board.undo();

        Move move(next_hand, eval);
        moves.push_back(move);

        movables ^= next_hand;
    }

    stable_sort(moves.begin(), moves.end(), MoveGreater());

    for (int i = 0; i < movables_size; i++) {
        sorted_moves[i] = moves[i].point;
    }

    return;
}

int NegaScoutAI::negaScout(Board& board, int limit, int alpha, int beta) {
    if (limit < 0 || board.isGameOver()) {
        leaves++;
        return evaluator->evaluate(board);
    }

    nodes++;

    uint64_t movables = board.getMovablePos();

    int t;
    if (movables == 0) {
        board.pass();
        t = -negaScout(board, limit, -beta, -alpha);
        board.undo();
        return t;
    }

    int a, b;
    a = alpha;
    b = beta;
    uint64_t next_hand;
    for (int i = 0; movables != 0; i++) {
        next_hand = movables & (-movables);

        board.move(next_hand);
        t = -negaScout(board, limit - 1, -b, -a);
        // cout << dec << "move1:" << Util::pointStr(next_hand, BOARD_SIZE) << ", t:" << t
        //     << ", alpha:" << alpha << ", beta:" << beta
        //     << ", a:" << a << ", b:" << b << ", limit:" << limit << endl;
        if (t > a && t < beta && i > 0 && limit > 2) {
            // 再探索
            a = -negaScout(board, limit - 1, -beta, -t);
            // cout << dec << "move2:" << Util::pointStr(next_hand, BOARD_SIZE) << ", t:" << t
            //     << ", alpha:" << alpha << ", beta:" << beta
            //     << ", a:" << a << ", b:" << b << ", limit:" << limit << endl;
        }
        board.undo();

        a = max(a, t);
        if (a >= beta)
            return a;

        b = a + 1;      // 新しいnull windowを設定

        movables ^= next_hand;
    }

    return a;
}

MonteCarloAI::MonteCarloAI() : AI() {
    trials = 1000;
}

MonteCarloAI::MonteCarloAI(const int trials) : AI() {
    this->trials = trials;
}

MonteCarloAI::~MonteCarloAI() {

}

std::uint64_t MonteCarloAI::act(Board& board) {
    uint64_t movables = board.getMovablePos();
    uint64_t best = 0;
    uint64_t next_hand = 0;

    if (movables == 0)
        return best;

    if (Util::bitCountInt8(movables) == 1)
        return movables;

    double eval;
    double max_eval = numeric_limits<int>::min();
    int trials_per_move = trials / Util::bitCountInt8(movables);

    if (is_debug) {
        cout << getClassName() << ", turns:" << dec << board.getTurns() << endl;
        cout << "trials:" << trials << ", trials_per_move:" << trials_per_move << endl;
    }

    while (movables != 0) {
        int win = 0;
        next_hand = movables & (-movables);

        board.move(next_hand);
        win = playout(board, trials_per_move);
        board.undo();

        eval = (double)win / trials_per_move;
        if (eval > max_eval) {
            best = next_hand;
            max_eval = eval;
        }

        if (is_debug) {
            cout << "win:" << win << ", trials_per_move:" << trials_per_move;
            cout << ", move:" << Util::pointStr(next_hand, BOARD_SIZE) << ", eval:" << fixed << setprecision(4) << max_eval << endl;
        }
        movables ^= next_hand;
    }

    if (is_debug)
        cout << ", best:" << Util::pointStr(best, BOARD_SIZE) << ", eval:" << fixed << setprecision(4) << max_eval << endl;
    return best;
}

int MonteCarloAI::playout(Board& board, const int trials) {
    std::uint64_t movables = 0;
    std::uint64_t next_hand = 0;
    int idx, diff;
    int win = 0;

    for (int i = 0; i < trials; i++) {
        Board tmp_board;
        tmp_board.copy(board);
        do {
            movables = tmp_board.getMovablePos();

            if (movables != 0) {
                idx = Util::random(Util::bitCountInt8(movables));
                for (int i = 0; i <= idx; i++) {
                    next_hand = movables & (-movables);
                    movables ^= next_hand;
                }
                tmp_board.move(next_hand);
            } else {
                tmp_board.pass();
            }
        } while (!tmp_board.isGameOver());

        diff = tmp_board.countDisc(board.getCurrentColor() ^ 1) - tmp_board.countDisc(board.getCurrentColor());
        if (diff > 0)
            win++;
    }

    return win;
}

MonteCarloTreeAI::MonteCarloTreeAI() : AI() {
    this->trials = 10000;
    this->exploration_param = std::sqrt(2);
    this->playout_threshold = 10;
}

MonteCarloTreeAI::MonteCarloTreeAI(const int trials, const double exploration_param, const int playout_threshold) : AI() {
    this->trials = trials;
    this->exploration_param = exploration_param;
    this->playout_threshold = playout_threshold;
}

MonteCarloTreeAI::~MonteCarloTreeAI() {

}

std::uint64_t MonteCarloTreeAI::act(Board& board) {
    uint64_t legal = board.getMovablePos();
    uint64_t best = 0;
    node.reserve(NODE_MAX);

    if (legal == 0)
        return legal;

    if (Util::bitCountInt8(legal) == 1)
        return legal;

    if (is_debug)
        cout << getClassName() << ", turns:" << dec << board.getTurns() << endl;

    int node_idx = createNode(legal);

    for (int i = 0; i < trials; i++)
        searchUTC(board, node_idx);

    int score_max = -99999;
    int win_rate_max = -99999;
    Node *pN = &node[node_idx];
    for (int i = 0; i < pN->child_num; i++) {
        if (pN->child[i].game_num > score_max ||
            (pN->child[i].game_num == score_max && pN->child[i].win_rate > win_rate_max)) {
            best = pN->child[i].point;
            score_max = pN->child[i].game_num;
        }

        if (is_debug)
            std::cout << Util::pointStr(pN->child[i].point, BOARD_SIZE)
                << ", win_rate:" << pN->child[i].win_rate << ", game_num:" << dec << pN->child[i].game_num << std::endl;
    }

    if (is_debug)
        cout << ", best:" << Util::pointStr(best, BOARD_SIZE) << ", eval:" << fixed << setprecision(4) << score_max << endl;

    node.shrink_to_fit();
    node_idx = 0;
    return best;
}

int MonteCarloTreeAI::searchUTC(Board& board, const int node_idx) {
    Node *pN = &node[node_idx];

    int child_idx = selectBestUCB(node_idx);
    Child *pC = &pN->child[child_idx];
    board.move(pC->point);
    int win;;
    if (pC->game_num <= playout_threshold) {
        MonteCarloAI ai;
        win = ai.playout(board, 1);
    } else {
        if (pC->next == NODE_EMPTY) {
            std::uint64_t legal = board.getMovablePos();
            pC->next = createNode(legal);
        }
        Board tmp_board;
        tmp_board.copy(board);
        //cout << "next:" << node.child[idx].next.child_game_sum << endl;
        if (searchUTC(tmp_board, pC->next) > 0)
            win = 0;
        else
            win = 1;
    }
    board.undo();

    pC->win_rate = (double)(pC->win_rate * pC->game_num + win) / (pC->game_num + 1);
    pC->game_num += 1;
    pN->child_game_sum += 1;
    // cout << "win_rate:" << pC->win_rate << ", game_num:" << pC->game_num
    //     << ", child_game_sum:" << pN->child_game_sum << endl;

    return win;
}

int MonteCarloTreeAI::selectBestUCB(const int node_idx) {
    Node *pN = &node[node_idx];
    double ucb_max = -999;
    double ucb;
    int best = 0;

    for (int i = 0; i < pN->child_num; i++) {
        if (pN->child[i].game_num == 0)
            ucb = 10000 + Util::random(0x7FFF);
        else
            ucb = pN->child[i].win_rate
                + exploration_param * std::sqrt(std::log(pN->child_game_sum) / pN->child[i].game_num);
        // if (pN->child[i].game_num != 0)
        //     cout << "ucb:" << ucb << ", win_rate:" << pN->child[i].win_rate
        //         << ", child_game_sum:" << pN->child_game_sum
        //         << ", game_num:" << pN->child[i].game_num
        //         << ", exploration_param:" << exploration_param
        //         << ", factor:" << exploration_param * std::sqrt(std::log(pN->child_game_sum) / pN->child[i].game_num)<< endl;

        // cout << "i:" << i << ", ucb_max:" << ucb_max << ", ucb:" << ucb << endl;
        if (ucb > ucb_max) {
            ucb_max = ucb;
            best = i;
        }
    }

    return best;
}

int MonteCarloTreeAI::createNode(std::uint64_t legal) {
    if (node_num >= NODE_MAX) {
        cerr << "node over" << endl;
        exit(0);
    }

    Node *pN = &node[node_num];
    pN->child_game_sum = 0;
    pN->child_num = 0;

    if (legal == 0) {
        pN->child[0].point = 0;
        pN->child[0].game_num = 0;
        pN->child[0].win_rate = 0;
        pN->child[0].next = NODE_EMPTY;
        pN->child_num++;
    } else {
        std::uint64_t point;
        int i;
        for (i = 0; legal != 0; i++) {
            point = legal & (-legal);

            pN->child[i].point = point;
            pN->child[i].game_num = 0;
            pN->child[i].win_rate = 0;
            pN->child[i].next = NODE_EMPTY;

            legal ^= point;
        }
        pN->child_num += i;
    }

    node_num++;
    return node_num - 1;
}
