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

class TableEvaluator : public Evaluator {
private:
    static const int Table[BOARD_SIZE * BOARD_SIZE];

public:
    static const std::string CLASS_NAME;

    int evaluate(const Board& board) {
        int eval = 0;
        std::uint64_t position[2];
        board.getPositions(position);
        std::uint64_t point = 0x8000000000000000ULL;
        Color self = board.getCurrentColor();
        Color enemy = self ^ 1;
        for (unsigned i = 0; point != 0; i++) {
            if ((position[self] & point) != 0)
                eval += Table[i];
            else if ((position[enemy] & point) != 0)
                eval -= Table[i];

            point = point >> 1;
        }

        return eval;
    }

    std::string getClassName() {
        return "TableEvaluator";
    }
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
