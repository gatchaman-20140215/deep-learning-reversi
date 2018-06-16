#ifndef EVALUATOR_H_INCLUDED
#define  EVALUATOR_H_INCLUDED

#include "Reversi.h"
#include "Board.h"
#include "Util.h"

#include <cstdint>
#include <cmath>
#include <string>

class Evaluator {
public:
    Evaluator() {}
    virtual ~Evaluator() {}

    virtual int evaluate(const Board& board) = 0;
    virtual std::string getClassName() = 0;
};

class WLDEvaluator : public Evaluator {
public:
    static const int WIN  =  1;
    static const int DRAW =  0;
    static const int LOSE = -1;

    int evaluate(const Board& board) {
        int discdiff =
            (board.countDisc(board.getCurrentColor()) - board.countDisc(board.getCurrentColor() ^ 1));

        if (discdiff > 0)
            return WIN;
        else if (discdiff < 0)
            return LOSE;
        else
            return DRAW;
    }

    std::string getClassName() {
        return "WLDEvaluator";
    }
};

class PerfectEvaluator : public Evaluator {
public:
    int evaluate(const Board& board) {
        int discdiff =
            (board.countDisc(board.getCurrentColor()) - board.countDisc(board.getCurrentColor() ^ 1));

        return discdiff;
    }

    std::string getClassName() {
        return "PerfectEvaluator";
    }
};

#endif
