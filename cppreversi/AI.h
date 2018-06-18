#include "Board.h"
#include "Evaluator.h"
#include <string>
#include <cstdint>
#include <vector>

class Evaluator;

class AI {
public:
    AI() {
        is_debug = true;
    }

    virtual std::uint64_t act(Board&) = 0;
    virtual std::string getClassName() = 0;

    void debug(bool is_debug) {
        this->is_debug = is_debug;
    }

protected:
    bool is_debug;

private:
    // コピー・代入は許可しない。
   AI(const AI&){}
   AI& operator=(const AI&) { return *this; }
};

class RandomAI : public AI {
public:
    RandomAI();
    virtual ~RandomAI() {}
    std::uint64_t act(Board& board);

    std::string getClassName() {
        return "RandomAI";
    }
};

class NegaMaxAI : public AI {
public:
    NegaMaxAI();
    NegaMaxAI(const int normal_depth);
    NegaMaxAI(const int normal_depth, const int wld_depth, const int perfect_depth);
    virtual ~NegaMaxAI();
    std::uint64_t act(Board& board);

    std::string getClassName() {
        return "NegaMaxAI";
    }

private:
    void sort(Board& board, std::uint64_t movables, int limit, std::uint64_t sorted_moves[], const int movables_size);
    int search(Board& board, int alpha, int beta, int depth);

    int presearch_depth;
    int normal_depth;
    int wld_depth;
    int perfect_depth;

    int depth_max;

    unsigned nodes;
    unsigned leaves;

    static const int MAX_SCORE = 99999;
    static const int MIN_SCORE = -99999;

    // 評価関数
    Evaluator* evaluator;

    struct Move {
        int eval;
        std::uint64_t point;

        Move() {
            point = 0;
            eval = 0;
        }

        Move(std::uint64_t p, int e) {
            point = p;
            eval = e;
        }
    };

    struct MoveGreater {
        bool operator() (const Move& lhs, const Move& rhs) {
            return (lhs.eval > rhs.eval);
        }
    };
};

class NegaScoutAI : public AI {
public:
    NegaScoutAI();
    NegaScoutAI(const int normal_depth);
    NegaScoutAI(const int normal_depth, const int wld_depth, const int perfect_depth);
    virtual ~NegaScoutAI();
    std::uint64_t act(Board& board);

    std::string getClassName() {
        return "NegaScoutAI";
    }

private:
    void sort(Board& board, std::uint64_t movables, int limit, std::uint64_t sorted_moves[], const int movables_size);
    int negaScout(Board& board, int limit, int alpha, int beta);

    int presearch_depth;
    int normal_depth;
    int wld_depth;
    int perfect_depth;

    unsigned nodes;
    unsigned leaves;

    static const int MAX_SCORE = 99999;
    static const int MIN_SCORE = -99999;

    // 評価関数
    Evaluator* evaluator;

    struct Move {
        int eval;
        std::uint64_t point;

        Move() {
            point = 0;
            eval = 0;
        }

        Move(std::uint64_t p, int e) {
            point = p;
            eval = e;
        }
    };

    struct MoveGreater {
        bool operator() (const Move& lhs, const Move& rhs) {
            return (lhs.eval > rhs.eval);
        }
    };
};

class MonteCarloAI : public AI {
public:
    MonteCarloAI();
    MonteCarloAI(const int trials);
    virtual ~MonteCarloAI();

    std::uint64_t act(Board& board);

    std::string getClassName() {
        return "MonteCarloAI";
    }

    int playout(Board& board, const int trials);

private:
    // 試行回数
    int trials;
};

class MonteCarloTreeAI : public AI {
public:
    MonteCarloTreeAI();
    MonteCarloTreeAI(const int trials, const double exploration_param, const int playout_threshold);
    virtual ~MonteCarloTreeAI();

    std::uint64_t act(Board& board);

    std::string getClassName() {
        return "MonteCarloTreeAI";
    }

private:
    // 試行回数
    int trials;
    // 探索係数
    double exploration_param;
    // プレイアウトを行うまでの試行回数
    int playout_threshold;

    static const int NODE_MAX = 1000000;
    static const int NODE_EMPTY = -1;

    struct Child {
        std::uint64_t point;
        int game_num;
        double win_rate;
        int next;
    };

    struct Node {
        int child_game_sum;
        int child_num;
        Child child[BOARD_SIZE * BOARD_SIZE];
    };

    std::vector<Node> node;
    // 登録ノード数
    int node_num = 0;

    int playout(Board& board, const int trials);
    int searchUTC(Board& board, const int node_idx);
    int selectBestUCB(const int node_idx);
    int createNode(std::uint64_t legal);
};
