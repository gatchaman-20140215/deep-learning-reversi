from pyreversi4.const import *

from abc import *

class Evaluator(object):
    __metaclass__ = ABCMeta

    def __init__(self):
        pass

    @classmethod
    @abstractmethod
    def uppercase(clas, name):
        raise NotImplementError()

    @abstractmethod
    def evaluate(self, board):
        raise NotImplementError()

class PerfectEvaluator(Evaluator):
    @classmethod
    def uppercase(cls, name):
        return name.upper() + ' by Impl'

    def evaluate(self, board):
        #print('current_color:{}'.format(board.current_color))
        #print('own:{0:016b}, enemy:{1:016b}'.format(board.position[0], board.position[1]))
        #print('own:{0:016b}, enemy:{1:016b}'.format(board.position[board.current_color], board.position[board.current_color ^ 1]))
        return bin(board.position[board.current_color]).count('1') - \
            bin(board.position[board.current_color ^ 1]).count('1')
