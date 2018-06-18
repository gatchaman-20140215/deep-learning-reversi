#ifndef MIDEVALUATOR_H_INCLUDED
#define MIDEVALUATOR_H_INCLUDED

#include "Evaluator.h"

#include <cstdint>

class MidEvaluator : public Evaluator {
private:
    // 重み係数を規定する構造体
    struct Weight {
        int mobility_w;
        int liberty_w;
        int stable_w;
        int wing_w;
        int x_move_w;
        int c_move_w;
    } EvalWieght;

    static const int MAX_SCORE = 99999;
    static const int MIN_SCORE = -99999;

    // 開放度を数える。
    int countLiberty(const Board& board, const Color color);
    // 確定石の個数を数える。
    int countStable(const Board& board, const Color color);
    // ウイングの個数を数える。
    int countWing(const Board& board, const Color color);
    // C打ちの個数を数える。
    int countCMove(const Board& board, const Color color);
    // X打ちの個数を数える。
    int countXMove(const Board& board, const Color color);

    // ウイングかどうか確認する。
    bool isWing(unsigned char self_edge, unsigned char enemy_edge);
    // 盤面左端の石の並びを取得する。
    unsigned char getLeftEdge(const Board& board, const Color color);
    // 盤面右端の石の並びを取得する。
    unsigned char getRightEdge(const Board& board, const Color color);
    // 盤面上端の石の並びを取得する。
    unsigned char getUpperEdge(const Board& board, const Color color);
    // 盤面下端の石の並びを取得する。
    unsigned char getLowerEdge(const Board& board, const Color color);
    // 指定した座標の周囲の座標を取得する。
    std::uint64_t getRound(const std::uint64_t point);

public:
    MidEvaluator();
    int evaluate(const Board& board);

    std::string getClassName() {
        return "MidEvaluator";
    }
};

#endif
