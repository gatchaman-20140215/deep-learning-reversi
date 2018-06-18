#include "Board.h"
#include <iostream>
#include <sstream>
#include <cstdint>
#include <iomanip>

Board::Board() {
    position.resize(2);
    position[BLACK] = 0x0000000810000000ULL;
    position[WHITE] = 0x0000001008000000ULL;

    turns = 0;
    currentColor = BLACK;
    movablePos = generateLeagal(currentColor);

    history_index = 0;
    Situation *pS = &history[history_index];
    pS->position[BLACK] = position[BLACK];
    pS->position[WHITE] = position[WHITE];
    pS->move = 0;
    pS->currentColor = currentColor;
    pS->turns = turns;
    pS->movablePos = movablePos;
}

Board::Board(const std::uint64_t black, const std::uint64_t white, const Color currentColor) {
    position.resize(2);
    position[BLACK] = black;
    position[WHITE] = white;

    turns = Util::bitCountInt8(position[BLACK] | position[WHITE]);
    this->currentColor = currentColor;
    movablePos = generateLeagal(currentColor);

    history_index = 0;
    Situation *pS = &history[history_index];
    pS->position[BLACK] = position[BLACK];
    pS->position[WHITE] = position[WHITE];
    pS->move = 0;
    pS->currentColor = currentColor;
    pS->turns = turns;
    pS->movablePos = movablePos;
}

bool Board::move(const std::uint64_t point) {
    if (point == 0)
        return pass();

    if ((movablePos & point) == 0)
        return false;

    std::uint64_t flipped = 0;

    const Color nextColor = currentColor ^ 1;
    flipped |= generateSomeFlipped(nextColor, MASK_EDGE, point, DIR_LEFT_OBRIQUE);
    flipped |= generateSomeFlipped(nextColor, MASK_EDGE, point, DIR_RIGHT_OBLIQUE);
    flipped |= generateSomeFlipped(nextColor, MASK_VERTICAL, point, DIR_HORIZONTAL);
    flipped |= generateSomeFlipped(nextColor, MASK_HORIZONTAL, point, DIR_VERTICAL);

    position[currentColor] = position[currentColor] | point | flipped;
    position[nextColor] = position[nextColor] ^ flipped;

    turns++;
    currentColor = nextColor;
    movablePos = generateLeagal(currentColor);

    history_index++;
    Situation *pS = &history[history_index];
    pS->position[BLACK] = position[BLACK];
    pS->position[WHITE] = position[WHITE];
    pS->move = point;
    pS->currentColor = currentColor;
    pS->turns = turns;
    pS->movablePos = movablePos;

    return true;
}

bool Board::pass() {
    if (movablePos != 0)
        return false;

    if (isGameOver())
        return false;

    currentColor = currentColor ^ 1;
    movablePos = generateLeagal(currentColor);

    history_index++;
    Situation *pS = &history[history_index];
    pS->position[BLACK] = position[BLACK];
    pS->position[WHITE] = position[WHITE];
    pS->move = 0;
    pS->currentColor = currentColor;
    pS->turns = turns;
    pS->movablePos = movablePos;

    return true;
}

bool Board::undo() {
    if (turns == 0)
        return false;

    Situation *pS = &history[history_index];
    pS->position[BLACK] = 0;
    pS->position[WHITE] = 0;
    pS->move = 0;
    pS->currentColor = 0;
    pS->turns = 0;
    pS->movablePos = 0;

    history_index--;
    pS = &history[history_index];
    position[BLACK] = pS->position[BLACK];
    position[WHITE] = pS->position[WHITE];
    currentColor = pS->currentColor;
    turns = pS->turns;
    movablePos = pS->movablePos;

    return true;
}

bool Board::isGameOver() const {
    if ((position[0] | position[1]) == 0xFFFFFFFFFFFFFFFFULL)
        return true;

    if (movablePos != 0)
        return false;

    if (generateLeagal(currentColor ^ 1) != 0)
        return false;

    return true;
}

std::vector<std::uint64_t> Board::generateHashKey() const {
    std::vector<std::uint64_t> hash(2);

    hash[BLACK] = position[BLACK];
    hash[WHITE] = position[WHITE];

    std::uint64_t black = position[BLACK];
    std::uint64_t white = position[WHITE];

    // 左90°回転（水平線に対する線対称）
    black = Board::rotateHorizontally(black);
    white = Board::rotateHorizontally(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左90°回転（垂直線に対する線対称）
    black = Board::rotateVertically(black);
    white = Board::rotateVertically(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左90°回転（水平線に対する線対称）
    black = Board::rotateHorizontally(black);
    white = Board::rotateHorizontally(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左斜線に対する線対称
    black = Board::transpose(black);
    white = Board::transpose(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左90°回転（水平線に対する線対称）
    black = Board::rotateHorizontally(black);
    white = Board::rotateHorizontally(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左90°回転（垂直線に対する線対称）
    black = Board::rotateVertically(black);
    white = Board::rotateVertically(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    // 左90°回転（水平線に対する線対称）
    black = Board::rotateHorizontally(black);
    white = Board::rotateHorizontally(white);
    if (black < hash[BLACK] || (black == hash[BLACK] && white < hash[WHITE])) {
        hash[BLACK] = black;
        hash[WHITE] = white;
    }

    return hash;
}

std::string Board::str() const {
    int i;
    std::uint64_t pos = 0x8000000000000000ULL;
    std::stringstream ss;

    if (currentColor == BLACK) {
        ss << "turn: ●" << std::endl;
    } else if (currentColor == WHITE) {
        ss << "turn: ○" << std::endl;
    }
    ss << "  a b c d e f g h " << std::endl;
    for (i = 1; i <= BOARD_SIZE * BOARD_SIZE; i++) {
        if (i % BOARD_SIZE == 1)
            ss << i / BOARD_SIZE + 1;

        if ((position[BLACK] & pos) != 0) {
            ss << "●";
        } else if ((position[WHITE] & pos) != 0) {
            ss << "○";
        } else {
            ss << "　";
        }

        if (i % BOARD_SIZE == 0)
            ss << std::endl;

        pos = pos >> 1;
    }
    ss << "0x" << std::setfill('0') << std::setw(16) << hex << position[BLACK] <<
        ", 0x" << std::setfill('0') << std::setw(16) << position[WHITE] << std::endl;
    ss << "----------------" << std::endl;
    return ss.str();
}

std::string Board::getMovablePosStr() const {
    std::stringstream ss;

    std::uint64_t next_hand;
    std::uint64_t movables = this->movablePos;
    while (movables != 0) {
        next_hand = movables & (-movables);
        ss << Util::pointStr(next_hand, BOARD_SIZE) << " ";
        movables ^= next_hand;
    }

    return ss.str();
}

int Board::countDisc(const Color color) const {
    if (color == BLACK || color == WHITE)
        return Util::bitCountInt8(position[color]);
    else
        return Util::bitCountInt8(~(position[BLACK] | position[WHITE]));
}

inline std::uint64_t Board::generateSomeFlipped(
    const unsigned nextColor, const std::uint64_t mask,
    const std::uint64_t point, const unsigned dir) {

    const std::uint64_t self = position[currentColor];
    const std::uint64_t maskedEnemy = position[nextColor] & mask;

    std::uint64_t flipped = 0;
    std::uint64_t m2, m3, m4, m5, m6;
    std::uint64_t m1 = point >> dir;

    if ((m1 & maskedEnemy) != 0) {
        if (((m2 = m1 >> dir) & maskedEnemy) == 0) {
            if ((m2 & self) != 0) {
                flipped |= m1;
            }
        } else if (((m3 = m2 >> dir) & maskedEnemy) == 0) {
            if ((m3 & self) != 0)
                flipped |= m1 | m2;
        } else if (((m4 = m3 >> dir) & maskedEnemy) == 0) {
            if ((m4 & self) != 0)
                flipped |= m1 | m2 | m3;
        } else if (((m5 = m4 >> dir) & maskedEnemy) == 0) {
            if ((m5 & self) != 0)
                flipped |= m1 | m2 | m3 | m4;
        } else if (((m6 = m5 >> dir) & maskedEnemy) == 0) {
            if ((m6 & self) != 0)
                flipped |= m1 | m2 | m3 | m4 | m5;
        } else {
            if (((m6 >> dir) & self) != 0)
                flipped |= m1 | m2 | m3 | m4 | m5 | m6;
        }
	}

    std::uint64_t m10, m11, m12, m13, m14;
	std::uint64_t m9 = point << dir;

    if ((m9 & maskedEnemy) != 0) {
        if (((m10 = m9 << dir) & maskedEnemy) == 0) {
            if ((m10 & self) != 0)
                flipped |= m9;
        } else if (((m11 = m10 << dir) & maskedEnemy) == 0) {
            if ((m11 & self) != 0)
                flipped |= m9 | m10;
        } else if (((m12 = m11 << dir) & maskedEnemy) == 0) {
            if ((m12 & self) != 0)
                flipped |= m9 | m10 | m11;
        } else if (((m13 = m12 << dir) & maskedEnemy) == 0) {
            if ((m13 & self) != 0)
                flipped |= m9 | m10 | m11 | m12;
        } else if (((m14 = m13 << dir) & maskedEnemy) == 0) {
            if ((m14 & self) != 0)
                flipped |= m9 | m10 | m11 | m12 | m13;
        } else {
            if (((m14 << dir) & self) != 0)
                flipped |= m9 | m10 | m11 | m12 | m13 | m14;
        }
    }

    return flipped;
}

std::uint64_t Board::generateLeagal(const unsigned currentColor) const {
    unsigned nextColor = currentColor ^ 1;

    std::uint64_t leagal = ~(position[currentColor] | position[nextColor])
                                & (generateSomeLeagal(position[currentColor], position[nextColor] & MASK_EDGE, DIR_LEFT_OBRIQUE)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_EDGE, DIR_RIGHT_OBLIQUE)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_VERTICAL, DIR_HORIZONTAL)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_HORIZONTAL, DIR_VERTICAL));

    return leagal;
}

inline std::uint64_t Board::generateSomeLeagal (
    const std::uint64_t self, const std::uint64_t maskedEnemy, const int dir) const {
    std::uint64_t flipped = ((self << dir) | (self >> dir)) & maskedEnemy;
    int i;
    for (i = 0; i < 6; i++) {
        flipped |= ((flipped << dir) | (flipped >> dir)) & maskedEnemy;
    }

    return flipped << dir | flipped >> dir;
}

std::uint64_t Board::rotateHorizontally(const std::uint64_t position) {
    std::uint64_t pos;
    pos = ((position >> 8) & 0x00FF00FF00FF00FFULL) | ((position << 8) & 0xFF00FF00FF00FF00ULL);
    pos = ((pos >> 16) & 0x0000FFFF0000FFFFULL) | ((pos << 16) & 0xFFFF0000FFFF0000ULL);
    return ((pos >> 32) & 0x00000000FFFFFFFFULL) | ((pos << 32) & 0xFFFFFFFF00000000ULL);
}

std::uint64_t Board::rotateVertically(const std::uint64_t position) {
    std::uint64_t pos;
    pos = ((position >> 1) & 0x5555555555555555ULL) | ((position << 1) & 0xAAAAAAAAAAAAAAAAULL);
    pos = ((pos >> 2) & 0x3333333333333333ULL) | ((pos << 2) & 0xCCCCCCCCCCCCCCCCULL);
    return ((pos >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((pos << 4) & 0xF0F0F0F0F0F0F0F0ULL);
}

std::uint64_t Board::transpose(const std::uint64_t postion) {
    std::uint64_t pos;
    pos = ((postion >> 8) & 0x00AA00AA00AA00AAULL) | ((postion >> 1) & 0x0055005500550055ULL)
            | ((postion << 1) & 0xAA00AA00AA00AA00ULL) | ((postion << 8) & 0x5500550055005500ULL);
    pos = ((pos >> 16) & 0x0000CCCC0000CCCCULL) | ((pos >> 2) & 0x0000333300003333ULL)
            | ((pos << 2) & 0xCCCC0000CCCC0000ULL) | ((pos << 16) & 0x3333000033330000ULL);
    return ((pos >> 32) & 0x00000000F0F0F0F0ULL) | ((pos >> 4) & 0x000000000F0F0F0FULL)
            | ((pos << 4) & 0xF0F0F0F000000000ULL) | ((pos << 32) & 0x0F0F0F0F00000000ULL);
}

std::vector<std::vector<std::uint64_t>> Board::rotatePosition(std::vector<std::uint64_t> list) {
    std::set<std::vector<std::uint64_t>> set;
    std::vector<std::uint64_t> rotated(list.size());

    int i = 0;
    for (std::uint64_t p : list) {
        rotated[i] = Board::rotateHorizontally(p);
        i++;
    }
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::rotateVertically(rotated[i]);
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::rotateHorizontally(rotated[i]);
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::transpose(rotated[i]);
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::rotateVertically(rotated[i]);
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::rotateHorizontally(rotated[i]);
    set.insert(rotated);

    for (unsigned i = 0; i < rotated.size(); i++)
        rotated[i] = Board::rotateVertically(rotated[i]);
    set.insert(rotated);

    std::vector<std::vector<std::uint64_t>> vec(set.begin(), set.end());

    return vec;
}


int Board::winner() const {
    if (!isGameOver())
        return NONE;

    int black = Util::bitCountInt8(position[BLACK]);
    int white = Util::bitCountInt8(position[WHITE]);
    if (black > white)
        return BLACK;
    else if (black < white)
        return WHITE;
    else
        return DRAW;
}
