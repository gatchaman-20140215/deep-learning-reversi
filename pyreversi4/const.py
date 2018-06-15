import string

BOARD_SIZE = 4
MAX_TURN = 16

EMPTY = -1
BLACK = 0
WHITE = 1
WALL = 2
DRAW = 2

MOVE = 2
TURN = 3
COLOR = 4
MOVABLE_POS = 5

MASK_EDGE = 0x0660
MASK_VERTICAL = 0x6666
MASK_HORIZONTAL	= 0x0FF0

DIR_LEFT_OBLIQUE = 5
DIR_RIGHT_OBLIQUE = 3
DIR_HORIZONTAL = 1
DIR_VERTICAL = 4

POSITION_NS_DICT = dict([(v, w) for (v, w) in zip(
    reversed([(1 << i) for i in range(16)]),
    ['{}{}'.format(j, i) for i in list(range(1, 5)) for j in list(string.ascii_lowercase)[0:4]])])
POSITION_NS_DICT[0] = 'pass'
POSITION_SN_DICT = { v: k for k, v in POSITION_NS_DICT.items() }
