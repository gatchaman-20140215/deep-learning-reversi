#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "Reversi.h"
#include "Util.h"
#include <cstdint>

class Board {
private:
    static const int _signature = 0x123f3f7c;

public:
    Board();
    Board(const std::uint16_t black, const std::uint16_t white, const Color currentColor);

    ~Board() {

    };

    // pointで指定された位置に石を打つ。
    bool move(const std::uint16_t point);
    // パスする。
    bool pass();
    // 1つ前の手の状態に戻す。
    bool undo();
    // ゲームが終了してるかどうかを判別する。
    bool isGameOver() const;

    // 盤面のハッシュ値を計算する。
    std::vector<std::uint16_t> generateHashKey() const;

    // 指定された石の個数を返す。
    int countDisc(const Color color) const;
    // 石を打てる場所を返す。
    std::uint16_t getMovablePos() const {
        return movablePos;
    }
    // 現在の手番の色を返す。
    Color getCurrentColor() const {
        return currentColor;
    }
    // 現在の手数を返す。
    int getTurns() const {
        return turns;
    }
    // 現在の盤を返す。
    void getPositions(std::uint16_t position[2]) const {
        position[BLACK] = this->position[BLACK];
        position[WHITE] = this->position[WHITE];
    }
    // 指定した色の石の配置を返す。
    std::uint16_t getPosition(const Color color) const {
        return position[color];
    }
    // 空マスの位置を返す。
    std::uint16_t getEmpty() const {
        return ~(position[BLACK] | position[WHITE]);
    }
    int countEmpty() const {
        return Util::bitCountInt2(~(position[BLACK] | position[WHITE]));
    }

    // 指定した場所の石の色を返す。
    Color getColor(std::uint16_t point) const {
        if ((position[BLACK] & point) != 0)
            return BLACK;
        else if ((position[WHITE] & point) != 0)
            return WHITE;
        else
            return EMPTY;
    }

    // 盤面を文字列化する。
    std::string str() const;
    // 石を打てる場所を文字列化したものを返す。
    std::string getMovablePosStr() const;

    bool operator==(const Board& board) const {
        return this->position[BLACK] == board.position[BLACK]
            && this->position[WHITE] == board.position[WHITE]
            && this->currentColor == board.currentColor;
    }

    int getHistorySize(Color color) const {
        int i;
        for (i = 0; i < MAX_TURNS * 2; i++) {
            if ((history[i].position[BLACK] | history[i].position[WHITE]) == 0)
                break;
            i++;
        }
        return i;
    }

    bool isLegal() const { return this->_sig == _signature; }

    bool isFull() const {
        return (position[BLACK] | position[WHITE]) == 0xFFFF;
    }

    // 勝者を取得する。
    int winner() const;

    // Boardオブジェクトをコピーする。
    void copy(const Board& src) {
        this->position[BLACK] = src.position[BLACK];
        this->position[WHITE] = src.position[WHITE];
        this->currentColor = src.currentColor;
        this->turns = src.turns;
        this->movablePos = src.movablePos;

        for (int i = 0; i < MAX_TURNS * 2; i++) {
            this->history[i].position[BLACK] = src.history[i].position[BLACK];
            this->history[i].position[WHITE] = src.history[i].position[WHITE];
            this->history[i].currentColor = src.history[i].currentColor;
            this->history[i].turns = src.history[i].turns;
            this->history[i].movablePos = src.history[i].movablePos;
        }
    }

    static std::uint16_t rotateHorizontally(std::uint16_t position);
    static std::uint16_t rotateVertically(std::uint16_t position);
    static std::uint16_t transpose(std::uint16_t postion);
    static std::vector<std::vector<std::uint16_t>> rotatePosition(std::vector<std::uint16_t> list);

    // 盤面
    std::vector<std::uint16_t> position;
    // 手数
    int turns;
    // 現在のプレイヤー
    Color currentColor;
    // 打てる場所
    std::uint16_t movablePos;

private:
    static const std::uint16_t MASK_EDGE		= 0x0660;
    static const std::uint16_t MASK_VERTICAL	= 0x6666;
    static const std::uint16_t MASK_HORIZONTAL	= 0x0FF0;

    static const unsigned DIR_LEFT_OBRIQUE	= 5;
    static const unsigned DIR_RIGHT_OBLIQUE	= 3;
    static const unsigned DIR_HORIZONTAL	= 1;
    static const unsigned DIR_VERTICAL		= 4;

    // 指定された方向に対し、必要な石を返す。
    std::uint16_t generateSomeFlipped(
        const unsigned nextColor, const std::uint16_t mask,
        const std::uint16_t point, const unsigned dir);
    // 合法手を生成する。
    std::uint16_t generateLeagal(const unsigned currentColor) const;
    // 指定された方向に対し、合法手を生成する。
    std::uint16_t generateSomeLeagal(
        const std::uint16_t self, const std::uint16_t maskedEnemy, const int dir) const;

    int _sig;

    struct Situation {
        std::uint16_t position[2];
        std::uint16_t move;
        Color currentColor;
        int turns;
        std::uint16_t movablePos;
    };
    // 履歴
    Situation history[MAX_TURNS * 2];
    // 履歴用インデックス
    int history_index;
};

#endif
