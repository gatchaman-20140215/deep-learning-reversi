#ifndef REVERSI_H_INCLUDED
#define REVERSI_H_INCLUDED

#include "Util.h"

#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <regex>

const int BOARD_SIZE = 4;
const int MAX_TURNS = 12;

typedef int Color;
const Color EMPTY = -1;
const Color BLACK = 0;
const Color WHITE = 1;
const Color WALL = 2;

const int MOVE = 2;     // 履歴テーブル用インデックス値
const int TURN = 3;     // 履歴テーブル用インデックス地

const int NONE = -1;
const int DRAW = 2;

#endif
