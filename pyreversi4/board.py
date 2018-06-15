from pyreversi4.const import *

import copy

class Board:
    def __init__(self, position=None, current_color=BLACK, current_turn=0):
        if position is None:
            self.position = [0x0240, 0x420]
            self.current_color = BLACK
        else:
            self.position = [position[BLACK], position[WHITE]]
            self.current_color = current_color

        self.movable_pos = self.generate_legal(self.current_color)

        self.current_turn = 0 if current_turn is None else current_turn
        self.history = [None] * (MAX_TURN + 16)
        self.history[self.current_turn] = [
            self.position[BLACK],
            self.position[WHITE],
            None,
            None,
            self.current_color,
            self.movable_pos
        ]

    def move(self, point):
        if point == 0: self.skip()
        if self.movable_pos & point == 0: return False

        next_color = self.current_color ^ 1
        flipped = self.generate_some_flipped(self.position[self.current_color], self.position[next_color] & MASK_EDGE, point, DIR_LEFT_OBLIQUE)
        flipped |= self.generate_some_flipped(self.position[self.current_color], self.position[next_color] & MASK_EDGE, point, DIR_RIGHT_OBLIQUE)
        flipped |= self.generate_some_flipped(self.position[self.current_color], self.position[next_color] & MASK_VERTICAL, point, DIR_HORIZONTAL)
        flipped |= self.generate_some_flipped(self.position[self.current_color], self.position[next_color] & MASK_HORIZONTAL, point, DIR_VERTICAL)

        self.position[self.current_color] = self.position[self.current_color] | point | flipped
        self.position[next_color] = self.position[next_color] ^ flipped

        self.history[self.current_turn][MOVE] = point
        self.current_turn += 1
        self.current_color = next_color
        self.movable_pos = self.generate_legal(self.current_color)
        self.history[self.current_turn] = [
            self.position[BLACK],
            self.position[WHITE],
            None,
            self.current_turn,
            self.current_color,
            self.movable_pos
        ]

        return True

    def skip(self):
        if self.movable_pos != 0: return False
        if self.is_game_over(): return False

        self.history[self.current_turn][MOVE] = 0
        self.current_color ^= 1
        self.current_turn += 1
        self.movable_pos = self.generate_legal(self.current_color)
        self.history[self.current_turn] = [
            self.position[BLACK],
            self.position[WHITE],
            None,
            self.current_turn,
            self.current_color,
            self.movable_pos
        ]

        return True

    def undo(self):
        if self.current_turn == 0: return False

        self.history[self.current_turn] = None
        self.current_color ^= 1
        self.current_turn -= 1
        self.position[BLACK] = self.history[self.current_turn][BLACK]
        self.position[WHITE] = self.history[self.current_turn][WHITE]
        self.movable_pos = self.history[self.current_turn][MOVABLE_POS]
        self.history[self.current_turn][MOVE] = None

        return True

    def is_game_over(self):
        if self.movable_pos != 0: return False
        if self.generate_legal(self.current_color ^ 1) != 0: return False

        return True

    def winner(self):
        if not self.is_game_over(): return None

        black_num = bin(self.position[BLACK]).count('1')
        white_num = bin(self.position[WHITE]).count('1')
        if black_num > white_num:
            return BLACK
        elif black_num < white_num:
            return WHITE
        else:
            return DRAW

    def is_board_full(self):
        return (self.position[BLACK] | self.position[WHITE]) == 0xFFFF

    def show_history(self):
        for h in self.history:
            if h is None: break
            board = Board(
                position=[h[BLACK], h[WHITE]],
                current_color=h[COLOR],
                current_turn=h[TURN]
            )
            print(board.to_string())
            print('move:{}'.format(POSITION_NS_DICT[h[MOVE]] if h[MOVE] is not None else 'pass'))

    def to_string(self):
        board_str = '手番：黒\n' if self.current_color == BLACK else '手番：白\n'
        board_str += '{0:#06x}, {1:#06x}\n'.format(self.position[BLACK], self.position[WHITE])
        board_str += ' a b c d\n'

        for y in range(BOARD_SIZE):
            board_str += '{}'.format(y + 1)
            for x in range(BOARD_SIZE):
                pos = 2 ** ((BOARD_SIZE - 1 - x) + (BOARD_SIZE - 1 - y) * BOARD_SIZE)
                if (self.position[BLACK] & pos) != 0:
                    board_str += '●'
                elif (self.position[WHITE] & pos) != 0:
                    board_str += '○'
                else:
                    board_str += '　'
            board_str += '\n'

        return board_str

    def copy(self):
        board = Board()
        board.position = copy.deepcopy(self.position)
        board.current_color = self.current_color
        board.current_turn = self.current_turn
        board.movable_pos = self.movable_pos
        board.history = copy.deepcopy(self.history)

        return board

    def movable_pos_array(self):
        a = []
        movable_pos = self.movable_pos
        while movable_pos != 0:
            act = movable_pos & (-movable_pos)
            a.append(act)
            movable_pos ^= act

        return a

    def generate_legal(self, turn=None):
        if turn is None: turn = self.current_color
        return (~(self.position[BLACK] | self.position[WHITE]) & 0xFFFF) \
            & (self.generate_some_legal(self.position[turn], self.position[turn ^ 1] & MASK_EDGE, DIR_LEFT_OBLIQUE) \
             | self.generate_some_legal(self.position[turn], self.position[turn ^ 1] & MASK_EDGE, DIR_RIGHT_OBLIQUE) \
             | self.generate_some_legal(self.position[turn], self.position[turn ^ 1] & MASK_VERTICAL, DIR_HORIZONTAL) \
             | self.generate_some_legal(self.position[turn], self.position[turn ^ 1] & MASK_HORIZONTAL, DIR_VERTICAL))

    def generate_some_legal(self, own, masked_enemy, direction):
        flipped = ((own << direction) | (own >> direction)) & masked_enemy
        for _ in range(BOARD_SIZE - 2):
            flipped |= ((flipped << direction) | (flipped >> direction)) & masked_enemy
        return (flipped << direction) | (flipped >> direction)

    def generate_some_flipped(self, own, masked_enemy, move, direction):
        left_enemy = (move << direction) & masked_enemy
        right_enemy = (move >> direction) & masked_enemy
        left_own = (own << direction) & masked_enemy
        right_own = (own >> direction) & masked_enemy

        return ((left_enemy & (right_own | (right_own >> direction))) | \
            ((left_enemy << direction) & right_own) | (right_enemy & (left_own | (left_own << direction))) | \
            ((right_enemy >> direction) & left_own))

    def hashkey(self):
        min_key = (self.position[BLACK] << 16) + self.position[WHITE]

        # 水平線に対する線対称
        black = Board.rotate_horizontally(self.position[BLACK])
        white = Board.rotate_horizontally(self.position[WHITE])
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 垂直線に対する線対称
        black = Board.rotate_vertically(black)
        white = Board.rotate_vertically(white)
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 水平線に対する線対称
        black = Board.rotate_horizontally(black)
        white = Board.rotate_horizontally(white)
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 左斜線に対する線対称
        black = Board.transpose(black)
        white = Board.transpose(white)
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 左90°回転（水平線に対する線対称）
        black = Board.rotate_horizontally(black)
        white = Board.rotate_horizontally(white)
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 左90°回転（垂直線に対する線対称）
        black = Board.rotate_vertically(black)
        white = Board.rotate_vertically(white)
        key = (black << 16) + white
        if key < min_key: min_key = key
        # 左90°回転（水平線に対する線対称）
        black = Board.rotate_horizontally(black)
        white = Board.rotate_horizontally(white)
        key = (black << 16) + white
        if key < min_key: min_key = key

        return min_key

    @staticmethod
    def rotate_horizontally(position):
        position = ((position >> 4) & 0x0F0F0F0F) | ((position << 4) & 0xF0F0F0F0)
        position = ((position >> 8) & 0x00FF00FF) | ((position << 8) & 0xFF00FF00)
        return position

    @staticmethod
    def rotate_vertically(position):
        position = ((position >> 1) & 0x55555555) | ((position << 1) & 0xAAAAAAAA)
        position = ((position >> 2) & 0x33333333) | ((position << 2) & 0xCCCCCCCC)
        return position

    @staticmethod
    def transpose(position):
        position = ((position >> 4) & 0x0A0A0A0A) | ((position >> 1) & 0x05050505) \
                 | ((position << 1) & 0xA0A0A0A0) | ((position << 4) & 0x50505050)
        position = ((position >> 8) & 0x00CC00CC) | ((position >> 2) & 0x00330033) \
                 | ((position << 2) & 0xCC00CC00) | ((position << 8) & 0x33003300)
        return position

        return ret

    @staticmethod
    def rotate_position(*args):
        ret = []

        rotated_v = [0] * len(args)
        for i, v in enumerate(args):
            rotated_v[i] = Board.rotate_horizontally(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.rotate_vertically(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.rotate_horizontally(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.transpose(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.rotate_horizontally(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.rotate_vertically(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        for i, v in enumerate(rotated_v):
            rotated_v[i] = Board.rotate_horizontally(v)
        if rotated_v not in ret:
            ret.append([v for v in rotated_v])

        return ret
