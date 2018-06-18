#include "Board.h"
#include <iostream>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <set>

Board::Board() {
    position.resize(2);
    position[BLACK] = 0x0240;
    position[WHITE] = 0x0420;

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

Board::Board(const std::uint16_t black, const std::uint16_t white, const Color currentColor) {
    position.resize(2);
    position[BLACK] = black;
    position[WHITE] = white;

    turns = Util::bitCountInt2(position[BLACK] | position[WHITE]);
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

bool Board::move(const std::uint16_t point) {
    if (point == 0)
        return pass();

    if ((movablePos & point) == 0)
        return false;

    std::uint16_t flipped = 0;

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
    if ((position[0] | position[1]) == 0xFFFF)
        return true;

    if (movablePos != 0)
        return false;

    if (generateLeagal(currentColor ^ 1) != 0)
        return false;

    return true;
}

std::vector<std::uint16_t> Board::generateHashKey() const {
    std::vector<std::uint16_t> hash(2);

    hash[BLACK] = position[BLACK];
    hash[WHITE] = position[WHITE];

    std::uint16_t black = position[BLACK];
    std::uint16_t white = position[WHITE];

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
    std::uint16_t pos = 0x8000;
    std::stringstream ss;

    if (currentColor == BLACK) {
        ss << u8"turn: ●" << std::endl;
    } else if (currentColor == WHITE) {
        ss << u8"turn: ○" << std::endl;
    }
    ss << u8"  a b c d " << std::endl;
    for (i = 1; i <= BOARD_SIZE * BOARD_SIZE; i++) {
        if (i % BOARD_SIZE == 1)
            ss << i / BOARD_SIZE + 1;

        if ((position[BLACK] & pos) != 0) {
            ss << u8"●";
        } else if ((position[WHITE] & pos) != 0) {
            ss << u8"○";
        } else {
            ss << u8"　";
        }

        if (i % BOARD_SIZE == 0)
            ss << std::endl;

        pos = pos >> 1;
    }
    ss << "0x" << std::setfill('0') << std::setw(4) << hex << position[BLACK] <<
        ", 0x" << std::setfill('0') << std::setw(4) << position[WHITE] << std::endl;
    ss << "----------------" << std::endl;
    return ss.str();
}

std::string Board::getMovablePosStr() const {
    std::stringstream ss;

    std::uint16_t next_hand;
    std::uint16_t movables = this->movablePos;
    while (movables != 0) {
        next_hand = movables & (-movables);
        ss << Util::pointStr(next_hand, BOARD_SIZE) << " ";
        movables ^= next_hand;
    }

    return ss.str();
}

int Board::countDisc(const Color color) const {
    if (color == BLACK || color == WHITE)
        return Util::bitCountInt2(position[color]);
    else
        return Util::bitCountInt2(~(position[BLACK] | position[WHITE]));
}

inline std::uint16_t Board::generateSomeFlipped(
    const unsigned nextColor, const std::uint16_t mask,
    const std::uint16_t point, const unsigned dir) {

    const std::uint16_t self = position[currentColor];
    const std::uint16_t maskedEnemy = position[nextColor] & mask;

    uint16_t leftEnemy = (point << dir) & maskedEnemy;
    uint16_t rightEnemy = (point >> dir) & maskedEnemy;
    uint16_t leftSelf = (self << dir) & maskedEnemy;
    uint16_t rightSelf = (self >> dir) & maskedEnemy;

    return ((leftEnemy & (rightSelf | (rightSelf >> dir))) |
        ((leftEnemy << dir) & rightSelf) | (rightEnemy & (leftSelf | (leftSelf << dir))) |
        ((rightEnemy >> dir) & leftSelf));
}

std::uint16_t Board::generateLeagal(const unsigned currentColor) const {
    unsigned nextColor = currentColor ^ 1;

    std::uint16_t leagal = ~(position[currentColor] | position[nextColor])
                                & (generateSomeLeagal(position[currentColor], position[nextColor] & MASK_EDGE, DIR_LEFT_OBRIQUE)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_EDGE, DIR_RIGHT_OBLIQUE)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_VERTICAL, DIR_HORIZONTAL)
                                |  generateSomeLeagal(position[currentColor], position[nextColor] & MASK_HORIZONTAL, DIR_VERTICAL));

    return leagal;
}

inline std::uint16_t Board::generateSomeLeagal (
    const std::uint16_t self, const std::uint16_t maskedEnemy, const int dir) const {
    std::uint16_t flipped = ((self << dir) | (self >> dir)) & maskedEnemy;
    int i;
    for (i = 0; i < 6; i++) {
        flipped |= ((flipped << dir) | (flipped >> dir)) & maskedEnemy;
    }

    return flipped << dir | flipped >> dir;
}

std::uint16_t Board::rotateHorizontally(std::uint16_t position) {
    position = ((position >> 4) & 0x0F0F0F0F) | ((position << 4) & 0xF0F0F0F0);
    return ((position >> 8) & 0x00FF00FF) | ((position << 8) & 0xFF00FF00);
}

std::uint16_t Board::rotateVertically(std::uint16_t position) {
    position = ((position >> 1) & 0x55555555) | ((position << 1) & 0xAAAAAAAA);
    return ((position >> 2) & 0x33333333) | ((position << 2) & 0xCCCCCCCC);
}

std::uint16_t Board::transpose(std::uint16_t postion) {
    postion = ((postion >> 4) & 0x0A0A0A0A) | ((postion >> 1) & 0x05050505)
            | ((postion << 1) & 0xA0A0A0A0) | ((postion << 4) & 0x50505050);
    return ((postion >> 8) & 0x00CC00CC) | ((postion >> 2) & 0x00330033)
            | ((postion << 2) & 0xCC00CC00) | ((postion << 8) & 0x33003300);
}

std::vector<std::vector<std::uint16_t>> Board::rotatePosition(std::vector<std::uint16_t> list) {
    std::set<std::vector<std::uint16_t>> set;
    std::vector<std::uint16_t> rotated(list.size());

    int i = 0;
    for (std::uint16_t p : list) {
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

    std::vector<std::vector<std::uint16_t>> vec(set.begin(), set.end());

    return vec;
}

int Board::winner() const {
    if (!isGameOver())
        return NONE;

    int black = Util::bitCountInt2(position[BLACK]);
    int white = Util::bitCountInt2(position[WHITE]);
    if (black > white)
        return BLACK;
    else if (black < white)
        return WHITE;
    else
        return DRAW;
}
