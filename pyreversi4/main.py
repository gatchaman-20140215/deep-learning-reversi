from pyreversi4.board import Board
from pyreversi4.player import *

import argparse
import logging

def get_player(player_str, color):
    if player_str == 'r':
        return PlayerRandom(color)
    elif player_str == 'n':
        return PlayerNegaMax(color, args.max_depth)
    elif player_str == 'mc':
        return PlayerMonteCarlo(color, args.try_num)
    elif player_str == 'mct':
        return PlayerMonteCarloTree(color, args.try_num)

# 引数の定義
parser = argparse.ArgumentParser()
parser.add_argument('--black', '-b', type=str, default='r', choices=['r', 'n', 'mc', 'mct'], help='black player')
parser.add_argument('--white', '-w', type=str, default='r', choices=['r', 'n', 'mc', 'mct'], help='white player')
parser.add_argument('--max_depth', type=int, default=9, help='NegaMax max search depth')
parser.add_argument('--try_num', type=int, default=1000, help='MonteCarlo max try number')
parser.add_argument('--log', default=None, help='log file path')
args = parser.parse_args()

# ログ出力の設定
logging.basicConfig(format='%(asctime)s\t%(levelname)s\t%(message)s',
    datefmt='%Y/%m/%d %H:%M:%S', filename=args.log, level=logging.DEBUG)

# 対局者の設定
players = [
    get_player(args.black, BLACK),
    get_player(args.white, WHITE)
]

# 対局の実施
logging.info('game start')
board = Board()
turn = BLACK

print(board.to_string())
while not board.is_game_over():
    players[turn].act(board)
    print(board.to_string())
    turn ^= 1

logging.info('winner:{}'.format(board.winner()))
logging.info('game end')
