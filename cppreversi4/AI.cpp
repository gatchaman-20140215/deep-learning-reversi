#include "AI.h"
#include "Board.h"
#include "Util.h"
#include <iomanip>

using namespace std;

RandomAI::RandomAI() : AI() {

}

std::uint16_t RandomAI::act(Board& board) {
    uint16_t movables = board.getMovablePos();
    uint16_t hand = 0;

    if (movables == 0) {
        return 0;
    }

    if (is_debug)
        cout << getClassName();

    int idx = Util::random(Util::bitCountInt2(movables)) + 1;
    for (int i = 0; i < idx; i++) {
        hand = movables & (-movables);
        movables ^= hand;
    }

    if (is_debug)
        cout << ", move:" << Util::pointStr(hand, BOARD_SIZE) << endl;
    return hand;
}

NegaMaxAI::NegaMaxAI() : AI() {
    evaluator = new PerfectEvaluator();
}

NegaMaxAI::~NegaMaxAI() {
    delete evaluator;
    evaluator = NULL;
}

std::uint16_t NegaMaxAI::act(Board& board) {
    nodes = 0;
    leaves = 0;

    uint16_t movables = board.getMovablePos();

    if (movables == 0)
        return 0;

    const int movables_size = Util::bitCountInt2(movables);
    if (movables_size == 1)
        return movables;

    if (is_debug)
        cout << getClassName() << "(" << evaluator->getClassName() << "), turns:" << dec << board.getTurns();

    int eval = MIN_SCORE;
    int evalMax = MIN_SCORE;
    uint16_t best = 0;
    uint16_t next_hand = 0;
    while (movables != 0) {
        next_hand = movables & (-movables);

        board.move(next_hand);
        eval = -search(board, MIN_SCORE, MAX_SCORE, 0);
        board.undo();

        if (eval > evalMax) {
            evalMax = eval;
            best = next_hand;
        }

        movables ^= next_hand;
    }

    if (is_debug) {
        cout << ", move:" << Util::pointStr(best, BOARD_SIZE) << ", eval:" << dec << evalMax;
        cout << ", leaves:" << leaves << ", nodes:" << nodes << endl;
    }

    return best;
}

int NegaMaxAI::search(Board& board, int alpha, int beta, int depth) {
    if (depth >= DEPTH_MAX || board.isFull()) {
        leaves++;
        return evaluator->evaluate(board);
    }

    nodes++;

    int eval;
    uint16_t movables = board.getMovablePos();

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

    uint16_t next_hand;
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

MonteCarloAI::MonteCarloAI() : AI() {
    trials = 1000;
}

MonteCarloAI::MonteCarloAI(const int trials) : AI() {
    this->trials = trials;
}

MonteCarloAI::~MonteCarloAI() {

}

std::uint16_t MonteCarloAI::act(Board& board) {
    uint16_t movables = board.getMovablePos();
    uint16_t best = 0;
    uint16_t next_hand = 0;

    if (movables == 0)
        return best;

    if (Util::bitCountInt2(movables) == 1)
        return movables;

    double eval;
    double max_eval = numeric_limits<int>::min();
    int trials_per_move = trials / Util::bitCountInt2(movables);

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
    std::uint16_t movables = 0;
    std::uint16_t next_hand = 0;
    int idx, diff;
    int win = 0;

    for (int i = 0; i < trials; i++) {
        Board tmp_board;
        tmp_board.copy(board);
        do {
            movables = tmp_board.getMovablePos();

            if (movables != 0) {
                idx = Util::random(Util::bitCountInt2(movables));
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

std::uint16_t MonteCarloTreeAI::act(Board& board) {
    uint16_t legal = board.getMovablePos();
    uint16_t best = 0;
    node.reserve(NODE_MAX);

    if (legal == 0)
        return legal;

    if (Util::bitCountInt2(legal) == 1)
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
    node_num = 0;
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
            std::uint16_t legal = board.getMovablePos();
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

int MonteCarloTreeAI::createNode(std::uint16_t legal) {
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
        std::uint16_t point;
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
