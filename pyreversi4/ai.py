from pyreversi4.const import *
from pyreversi4.util import *

from abc import *
import random
import math

class AI(object):
    __metaclass__ = ABCMeta

    MAX_SCORE = 99999
    MIN_SCORE = -99999

    def __init__(self):
        pass

    @classmethod
    @abstractmethod
    def uppercase(cls, name):
        raise NotImplementError()

    @abstractmethod
    def move(self, board):
        raise NotImplementError()

class RandomAI(AI):
    @classmethod
    def uppercase(cls, name):
        return name.upper() + ' by Impl'

    def __init__(self):
        pass

    def act(self, board):
        legal = board.movable_pos
        legals = list(filter(lambda p: (legal & p) != 0, POSITION_NS_DICT.keys()))
        if len(legals) != 0:
            return random.choice(legals)
        else:
            return 0

    def move(self, board):
        act = self.act(board)
        if act != 0:
            board.move(act)
        else:
            board.skip()

class NegaMaxAI(AI):
    @classmethod
    def uppercase(cls, name):
        return name.upper() + ' by Impl'

    def __init__(self, evaluator, depth_max=15):
        self.evaluator = evaluator
        self.depth_max = depth_max
        self.node_count = 0
        self.leaf_count = 0

    def act(self, board):
        # timer = Timer()
        # timer.start()

        legal = board.movable_pos
        if legal == 0:
            return 0

        if bin(legal).count('1') == 1:
            return legal

        score, score_max = self.MIN_SCORE, self.MIN_SCORE
        best = 0
        while legal != 0:
            act = legal & (-legal)
            board.move(act)
            score = -self.search(board, self.MIN_SCORE, self.MAX_SCORE, 0)
            board.undo()

            if score > score_max:
                score_max = score
                best = act

            #print('move:{}, eval:{}'.format(POSITION_NS_DICT[act], score))
            legal ^= act

        # print('move:{}, eval:{}'.format(POSITION_NS_DICT[best], score_max))
        # print('leaf:{}, node:{}, time:{}'.format(self.leaf_count, self.node_count, timer.stop()))
        return best

    def move(self, board):
        act = self.act(board)
        if act != 0:
            board.move(act)
        else:
            board.skip()

    def search(self, board, alpha, beta, depth):
        self.node_count += 1

        if depth >= self.depth_max or board.is_board_full():
            self.leaf_count += 1
            return self.evaluator.evaluate(board)

        legal = board.movable_pos
        if legal == 0:
            if board.is_game_over():
                self.leaf_count += 1
                return self.evaluator.evaluate(board)
            else:
                board.skip()
                score = -self.search(board, -beta, -alpha, depth + 1)
                board.undo()
                return score

        while legal != 0:
            act = legal & (-legal)
            board.move(act)
            score = -self.search(board, -beta, -alpha, depth + 1)
            board.undo()

            if score >= beta:
                return score

            if score > alpha:
                alpha = score

            legal ^= act

        return alpha

class MonteCarloAI(AI):
    @classmethod
    def uppercase(cls, name):
        return name.upper() + ' by Impl'

    def __init__(self, try_num):
        self.try_num = try_num

    def act(self, board):
        legal = board.movable_pos
        if legal == 0:
            return 0

        if bin(legal).count('1') == 1:
            return legal

        score_max = 0
        best = 0
        while legal != 0:
            act = legal & (-legal)
            board.move(act)
            win = self.playout(board)
            board.undo()
            score = win / self.try_num
            print('move:{0}, score:{1:06.4f}'.format(POSITION_NS_DICT[act], score))

            if score > score_max:
                score_max = score
                best = act
            legal ^= act

        return best

    def move(self, board):
        act = self.act(board)
        if act == 0:
            board.skip()
        else:
            board.move(act)

    def playout(self, board):
        turn = board.current_color
        players = [RandomAI(), RandomAI()]
        win = 0
        for _ in range(self.try_num):
            tmp_board = board.copy()
            while not tmp_board.is_game_over():
                players[turn].move(tmp_board)
                turn ^= 1
            diff = bin(tmp_board.position[board.current_color ^ 1]).count('1') - bin(tmp_board.position[board.current_color]).count('1')
            if diff > 0:
                win += 1

        #print('win:{}, lose:{}, draw{}'.format(win, lose, draw))
        return win

class MonteCarloTreeAI(AI):
    @classmethod
    def uppercase(cls, name):
        return name.upper() + ' by Impl'

    def __init__(self, try_num, exploration_param=math.sqrt(2), playout_threshold=10):
        self.try_num = try_num
        self.exploration_param = exploration_param
        self.playout_threshold = playout_threshold
        self.montecarlo_ai = MonteCarloAI(try_num=1)

    def act(self, board):
        legal = board.movable_pos
        if legal == 0:
            return 0

        if bin(legal).count('1') == 1:
            return legal

        node = self.create_node(legal)

        for _ in range(self.try_num):
            self.search_uct(board, node)

        score_max = -99999
        for c in node['child']:
            if c['game_num'] > score_max:
                best = c
                score_max = c['game_num']
            print('{0}, win_rate:{1:06.4f}, game_num:{2}'.format(POSITION_NS_DICT[c['point']], c['win_rate'], c['game_num']))

        return best['point']

    def move(self, board):
        act = self.act(board)
        if act == 0:
            board.skip()
        else:
            board.move(act)

    def search_uct(self, board, node):
        c = self.select_best_ucb(node)
        board.move(c['point'])
        if c['game_num'] <= self.playout_threshold :
            win = self.montecarlo_ai.playout(board)
        else:
            if c['next'] is None:
                legal = board.movable_pos
                c['next'] = self.create_node(legal)
            win = 0 if self.search_uct(board.copy(), c['next']) > 0 else 1
        board.undo()

        c['win_rate'] = (c['win_rate'] * c['game_num'] + win) / (c['game_num'] + 1)
        c['game_num'] += 1
        node['child_game_sum'] += 1
        return win

    def select_best_ucb(self, node):
        ucb_max = -999
        for c in node['child']:
            if c['game_num'] == 0:
                ucb = 10000 + random.randint(0, 0x7FFF)
            else:
                ucb = c['win_rate'] + self.exploration_param * math.sqrt(math.log(node['child_game_sum']) / c['game_num'])
            # print('ucb:{}, win_rate:{}, child_game_sum:{}, game_num:{}, C:{}'.format(
            #     ucb, c['win_rate'], node['child_game_sum'], c['game_num'], self.exploration_param))

            if ucb > ucb_max:
                ucb_max = ucb
                select = c

        return select

    def create_node(self, legal):
        if legal == 0:
            node = { 'child_game_sum': 0, 'child': [
                {
                    'point': 0,
                    'game_num': 0,
                    'win_rate': 0,
                    'next': None
                }
            ]}
        else:
            node = { 'child_game_sum': 0, 'child': [None] * bin(legal).count('1') }
            i = 0
            while legal != 0:
                point = legal & (-legal)
                node['child'][i] = {
                    'point': point,
                    'game_num': 0,
                    'win_rate': 0,
                    'next': None
                }
                legal ^= point
                i += 1

        return node
