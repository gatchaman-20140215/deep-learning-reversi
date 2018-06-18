#include "MidEvaluator.h"
#include "Util.h"
#include "Reversi.h"
#include "Board.h"

#include <cstdint>
#include <iomanip>

MidEvaluator::MidEvaluator() {
    // 重み係数の設定
    EvalWieght.mobility_w =   67;
    EvalWieght.liberty_w  =  -13;
    EvalWieght.stable_w   =  101;
    EvalWieght.wing_w     = -308;
    EvalWieght.x_move_w   = -449;
    EvalWieght.c_move_w   = -552;
}

int MidEvaluator::evaluate(const Board& board) {
    Color ownColor = board.getCurrentColor();
    Color enemyColor = ownColor ^ 1;

    if (board.countDisc(ownColor) == 0)
        return MIN_SCORE;
    else if (board.countDisc(enemyColor) == 0)
        return MAX_SCORE;

    // 着手可能手数を取得
    int mobility = Util::bitCountInt8(board.getMovablePos());
    // 開放度を取得
    int liberty_own = countLiberty(board, ownColor);
    int liberty_enemy = countLiberty(board, enemyColor);
    // ウイングの数を取得
    int wing_own = countWing(board, ownColor);
    int wing_enemy = countWing(board, enemyColor);
    // 確定石の個数を取得
    int fixed_piece_own = countStable(board, ownColor);
    int fixed_piece_enemy = countStable(board, enemyColor);
    // C打ちの個数を取得
    int c_move_own = countCMove(board, ownColor);
    int c_move_enemy = countCMove(board, enemyColor);
    // X打ちの個数を取得
    int x_move_own = countXMove(board, ownColor);
    int x_move_enemy = countXMove(board, enemyColor);

    // std::cout << "mobility:" << mobility << std::endl;
    // std::cout << "liberty_own:" << liberty_own
    //             << ", liberty_enemy:" << liberty_enemy << std::endl;
    // std::cout << "fixed_piece_own:" << fixed_piece_own
    //             << ", fixed_piece_enemy:" << fixed_piece_enemy << std::endl;
    // std::cout << "wing_own:" << wing_own
    //             << ", wing_enemy:" << wing_enemy << std::endl;
    // std::cout << "x_move_own:" << x_move_own
    //             << ", x_move_enemy:" << x_move_enemy << std::endl;
    // std::cout << "c_move_own:" << c_move_own
    //             << ", c_move_enemy:" << c_move_enemy << std::endl;

    return mobility * EvalWieght.mobility_w
            + (liberty_own - liberty_enemy) * EvalWieght.liberty_w
            + (fixed_piece_own - fixed_piece_enemy) * EvalWieght.stable_w
            + (wing_own - wing_enemy) * EvalWieght.wing_w
            + (x_move_own - x_move_enemy) * EvalWieght.x_move_w
            + (c_move_own - c_move_enemy) * EvalWieght.c_move_w;
}

int MidEvaluator::countLiberty(const Board& board, const Color color) {
    std::uint64_t self = board.getPosition(color);
	std::uint64_t empty = board.getEmpty();
	std::uint64_t point;
	int liberty = 0;

	while (self != 0)
	{
		point = self & (-self);
		liberty += Util::bitCountInt8(empty & getRound(point));
        // std::cout << "point:" << std::setfill('0') << std::setw(16) << hex << point
        //     << ", liberty:" << dec << liberty << std::endl;
		self ^= point;
	}

	return liberty;
}

int MidEvaluator::countStable(const Board& board, const Color color) {
	const unsigned int mask = 0x000000FF;
	const std::uint64_t mask_corner = 0x8100000000000081ULL;

	unsigned char edge;
	int count = 0;

	// 右辺
	edge = ~getRightEdge(board, color);
	count += Util::bitCountInt2(((edge & (-edge)) - 1) & mask);
	if ((edge & 0x80) == 0 && edge != 0) {
		edge = edge | (edge >> 1);
		edge = edge | (edge >> 2);
		edge = edge | (edge >> 2);
		count += Util::bitCountInt2((~edge) & mask);
	}

	// 左辺
	edge = ~getLeftEdge(board, color);
	count += Util::bitCountInt2(((edge & (-edge)) - 1) & mask);
	if ((edge & 0x80) == 0 && edge != 0) {
		edge = edge | (edge >> 1);
		edge = edge | (edge >> 2);
		edge = edge | (edge >> 2);
		count += Util::bitCountInt2((~edge) & mask);
	}

	// 上辺
	edge = ~getUpperEdge(board, color);
	count += Util::bitCountInt2(((edge & (-edge)) - 1) & mask);
	if ((edge & 0x80) == 0 && edge != 0) {
		edge = edge | (edge >> 1);
		edge = edge | (edge >> 2);
		edge = edge | (edge >> 2);
		count += Util::bitCountInt2((~edge) & mask);
	}

	// 下辺
	edge = ~getLowerEdge(board, color);
	count += Util::bitCountInt2(((edge & (-edge)) - 1) & mask);
	if ((edge & 0x80) == 0 && edge != 0) {
		edge = edge | (edge >> 1);
		edge = edge | (edge >> 2);
		edge = edge | (edge >> 2);
		count += Util::bitCountInt2((~edge) & mask);
	}

	count -= Util::bitCountInt8(board.getPosition(color) & mask_corner);
	return count;
}

int MidEvaluator::countWing(const Board& board, const Color color) {
    int count = 0;
	Color enemy_color = color ^ 1;

	if (isWing(getUpperEdge(board, color), getUpperEdge(board, enemy_color)))
		count++;
	if (isWing(getLowerEdge(board, color), getLowerEdge(board, enemy_color)))
		count++;
	if (isWing(getLeftEdge(board, color), getLeftEdge(board, enemy_color)))
		count++;
	if (isWing(getRightEdge(board, color), getRightEdge(board, enemy_color)))
		count++;

	return count;
}

int MidEvaluator::countCMove(const Board& board, const Color color) {
    int count = 0;
	Color enemy_color = color ^ 1;

	// 上辺
	unsigned char self_line = getUpperEdge(board, color);
	unsigned char enemy_line = getUpperEdge(board, enemy_color);
	unsigned char line = self_line | enemy_line;
	if (((line & 0x80) == 0) && ((self_line & 0x40) != 0))
		count++;
	if (((line & 0x01) == 0) && ((self_line & 0x02) != 0))
		count++;

	// 下辺
	self_line = getLowerEdge(board, color);
	enemy_line = getLowerEdge(board, enemy_color);
	line = self_line | enemy_line;
	if (((line & 0x80) == 0) && ((self_line & 0x40) != 0))
		count++;
	if (((line & 0x01) == 0) && ((self_line & 0x02) != 0))
		count++;

	// 左辺
	self_line = getLeftEdge(board, color);
	enemy_line = getLeftEdge(board, enemy_color);
	line = self_line | enemy_line;
	if (((line & 0x80) == 0) && ((self_line & 0x40) != 0))
		count++;
	if (((line & 0x01) == 0) && ((self_line & 0x02) != 0))
		count++;

	// 右辺
	self_line = getRightEdge(board, color);
	enemy_line = getRightEdge(board, enemy_color);
	line = self_line | enemy_line;
	if (((line & 0x80) == 0) && ((self_line & 0x40) != 0))
		count++;
	if (((line & 0x01) == 0) && ((self_line & 0x02) != 0))
		count++;

	return count;
}

int MidEvaluator::countXMove(const Board& board, const Color color) {
    int count = 0;
    std::uint64_t position[2];
    board.getPositions(position);
	std::uint64_t corner = (position[color] & 0x8100000000000081ULL) | (position[color ^ 1] & 0x8100000000000081);

	// 左上
	if (((corner & 0x8000000000000000ULL) == 0) && ((position[color] & 0x0040000000000000ULL) != 0))
		count++;
	// 右上
	if (((corner & 0x0100000000000000ULL) == 0) && ((position[color] & 0x0002000000000000ULL) != 0))
		count++;
	// 左上
	if (((corner & 0x0000000000000080ULL) == 0) && ((position[color] & 0x0000000000004000ULL) != 0))
		count++;
	// 右下
	if (((corner & 0x0000000000000001ULL) == 0) && ((position[color] & 0x0000000000000200ULL) != 0))
		count++;

	return count;
}

bool MidEvaluator::isWing(unsigned char self_edge, unsigned char enemy_edge) {
    if (((self_edge | enemy_edge) & 0x81) != 0) {
		return false;
	}

	return (self_edge == 0b01111100 || self_edge == 0b00111110);
}

unsigned char MidEvaluator::getLeftEdge(const Board& board, const Color color) {
    const std::uint64_t mask = 0x8000000000000000ULL;
    std::uint64_t position = board.getPosition(color);

	return (position & mask) >> 56
			| (position & (mask >>  8)) >> (48 + 1)
			| (position & (mask >> 16)) >> (40 + 2)
			| (position & (mask >> 24)) >> (32 + 3)
			| (position & (mask >> 32)) >> (24 + 4)
			| (position & (mask >> 40)) >> (16 + 5)
			| (position & (mask >> 48)) >> ( 8 + 6)
			| (position & (mask >> 56)) >> ( 0 + 7);
}

unsigned char MidEvaluator::getRightEdge(const Board& board, const Color color) {
    const std::uint64_t mask = 0x0100000000000000ULL;
    std::uint64_t position = board.getPosition(color);

	return (position & mask) >> (56 - 7)
			| (position & (mask >>  8)) >> (48 - 6)
			| (position & (mask >> 16)) >> (40 - 5)
			| (position & (mask >> 24)) >> (32 - 4)
			| (position & (mask >> 32)) >> (24 - 3)
			| (position & (mask >> 40)) >> (16 - 2)
			| (position & (mask >> 48)) >> ( 8 - 1)
			| (position & (mask >> 56)) >> ( 0 - 0);
}

unsigned char MidEvaluator::getUpperEdge(const Board& board, const Color color) {
    return board.getPosition(color) >> 56;
}

unsigned char MidEvaluator::getLowerEdge(const Board& board, const Color color) {
    return board.getPosition(color);
}

std::uint64_t MidEvaluator::getRound(const std::uint64_t point) {
    std::uint64_t around;

	if ((point & 0x8080808080808080ULL) != 0) {
		around = point |  (point >> 1);
		around = around | (point << 8) | (point << 7);
		around = around | (point >> 8) | (point >> 9);
	} else if ((point & 0x0101010101010101ULL) != 0) {
		around = point |  (point << 1);
		around = around | (point << 9) | (point << 8);
		around = around | (point >> 7) | (point >> 8);
	} else {
		around = point | (point << 1) | (point >> 1);
		around = around | (point << 9) | (point << 8) | (point << 7);
		around = around | (point >> 7) | (point >> 8) | (point >> 9);
	}

	return around;
}
