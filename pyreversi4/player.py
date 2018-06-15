from pyreversi4.ai import *
from pyreversi4.evaluator import *

from abc import *

class Player(object):
    __metaclass__ = ABCMeta

    def __init__(self):
        pass

    @abstractmethod
    def act(self, board):
        raise NotImplementError()

class PlayerRandom(Player):
    def __init__(self, color, name='Random'):
        self.color = color
        self.name = name

    def act(self, board):
        ai = RandomAI()
        ai.move(board)

class PlayerNegaMax(Player):
    def __init__(self, color, name='NegaMax', max_depth=15):
        self.color = color
        self.name = name
        self.max_depth = max_depth

    def act(self, board):
        evaluator = PerfectEvaluator()
        ai = NegaMaxAI(evaluator, self.max_depth)
        ai.move(board)

class PlayerMonteCarlo(Player):
    def __init__(self, color, name='MonteCarlo', try_num=1000):
        self.color = color
        self.name = name
        self.try_num = try_num

    def act(self, board):
        ai = MonteCarloAI(self.try_num)
        ai.move(board)

class PlayerMonteCarloTree(Player):
    def __init__(self, color, name='MonteCarloTree', try_num=1000):
        self.color = color
        self.name = name
        self.try_num = try_num

    def act(self, board):
        ai = MonteCarloTreeAI(self.try_num)
        ai.move(board)
