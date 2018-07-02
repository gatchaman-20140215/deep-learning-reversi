from cpp_reversi4 import Board, RandomAI, NegaMaxAI, MonteCarloTreeAI
from cpp_reversi4 import BLACK, WHITE, DRAW, NONE

import random
import math

import argparse
import logging
from time import sleep

mask = 0x8000
IDX_TO_POS = {0: 0}
POS_TO_IDX = {0: 0}
for i in range(16):
    IDX_TO_POS[i + 1] = mask
    POS_TO_IDX[mask] = i + 1
    mask = mask >> 1

class PyBoard(Board):
    def state(self):
        # return (self.current_color << 32) + (self.position[BLACK] << 16) + self.position[WHITE]
        return (self.position[self.current_color] << 16) + self.position[self.current_color ^ 1]

    def acts(self):
        movable_pos = self.movable_pos
        if movable_pos == 0:
            acts = [0]
        else:
            acts = []
            for pos in POS_TO_IDX:
                if (movable_pos & pos) != 0:
                    acts.append(POS_TO_IDX[pos])
        return acts

class GameOrganizer:
    def __init__(self, player_b, player_w, num_play=1, show_board=True, show_result=True, stat=100):
        self.player_b = player_b
        self.player_w = player_w
        self.num_won = {player_b.my_color:0, player_w.my_color:0, DRAW:0}
        self.num_won_prev = {player_b.my_color:0, player_w.my_color:0, DRAW:0}
        self.num_play = num_play
        self.players = [self.player_b, self.player_w]
        self.disp = show_board
        self.show_result = show_result
        # self.player_turn = self.players[random.randrange(2)]
        self.num_played = 0
        self.stat = stat

    def progress(self):
        while self.num_played < self.num_play:
            turn = BLACK
            self.players[BLACK].current_color = BLACK
            self.players[WHITE].current_color = WHITE
            board = PyBoard()
            # board = PyBoard(0x7250, 0x8ca2, BLACK)
            # board = PyBoard(0x104c, 0xeeb2, BLACK)
            if self.disp:
                print(board.to_string())
            while True:
                if self.disp:
                    print('Trun is {}\n'.format(self.players[turn].name))
                act = self.players[turn].act(board)
                result = board.move(act)
                if self.disp:
                    print(board.to_string())
                if result is False:
                    for p in self.players:
                        p.get_game_result(board, missed=True)
                    print('Invalid Move!')
                    self.num_won[self.players[turn ^ 1].my_color] += 1
                    break

                winner = board.winner()
                if winner != NONE:
                    for p in self.players:
                        p.get_game_result(board, missed=False)
                    if winner == DRAW:
                        if self.show_result:
                            print('Draw Game')
                        self.num_won[DRAW] += 1
                    elif winner == self.players[turn].current_color:
                        if self.show_result:
                            print('Winner:{}'.format(self.players[turn].name))
                        self.num_won[self.players[turn].my_color] += 1
                    else:
                        if self.show_result:
                            print('Winner:{}'.format(self.players[turn ^ 1].name))
                        self.num_won[self.players[turn ^ 1].my_color] += 1
                    break
                else:
                    # self.switch_player()
                    turn ^= 1
                    self.players[turn].get_game_result(board, missed=False, last_move=act)

            self.num_played += 1
            if self.num_played % self.stat == 0 or self.num_played == self.num_play:
                b_win_sum = self.num_won[self.player_b.my_color] - self.num_won_prev[self.player_b.my_color]
                w_win_sum = self.num_won[self.player_w.my_color] - self.num_won_prev[self.player_w.my_color]
                draw_sum = self.num_won[DRAW] - self.num_won_prev[DRAW]
                print('{}:{},{}:{},DRAW:{} {:.4f},{},{},{}'.format(
                    self.player_b.name, self.num_won[self.player_b.my_color],
                    self.player_w.name, self.num_won[self.player_w.my_color],
                    self.num_won[DRAW],
                    w_win_sum / (b_win_sum + w_win_sum + draw_sum), b_win_sum, w_win_sum, draw_sum))
                self.num_won_prev[self.player_b.my_color] = self.num_won[self.player_b.my_color]
                self.num_won_prev[self.player_w.my_color] = self.num_won[self.player_w.my_color]
                self.num_won_prev[DRAW] = self.num_won[DRAW]

                if logging.getLogger().isEnabledFor(logging.DEBUG):
                    for p in self.players:
                        if hasattr(p, 'q'):
                            # print(p.q)
                            print('len:{},{}'.format(len(p.q), len([p.q[k] for k in p.q if p.q[k] != 1])))
                            # for k in p.q:
                            #     print('{}, {}'.format(k[1], p.q[k]))
                            #     print(Board(k[0] >> 16, k[0] & 0xFFFF, BLACK).to_string())

            self.players.reverse()

    def switch_player(self):
        if self.player_turn == self.player_b:
            self.player_turn = self.player_w
        else:
            self.player_turn = self.player_b

class PlayerRandom:
    def __init__(self, color, name='Random'):
        self.name = name
        self.my_color = color
        self.current_color = color
        self.ai = RandomAI()
        self.ai.debug(False)

    def act(self, board):
        return self.ai.act(board)

    def get_game_result(self, board, missed, last_move=None):
        pass

class PlayerNegaMax:
    def __init__(self, color, name='NegaMax'):
        self.name = name
        self.my_color = color
        self.current_color = color
        self.ai = NegaMaxAI()
        self.ai.debug(False)

    def act(self, board):
        return self.ai.act(board)

    def get_game_result(self, board, missed, last_move=None):
        pass

class PlayerMonteCarloTree:
    def __init__(self, color, name='MonteCarlo'):
        self.name = name
        self.my_color = color
        self.current_color = color
        self.ai = MonteCarloTreeAI(1000, math.sqrt(2), 10)
        self.ai.debug(False)

    def act(self, board):
        return self.ai.act(board)

    def get_game_result(self, board, missed, last_move=None):
        pass

class PlayerHuman:
    def __init__(self, color, name='Human'):
        self.name = name
        self.my_color = color
        self.current_color = color

    def act(self, board):
        valid = False
        while not valid:
            try:
                act = input('Where would you like to place {} (0-16)? '.format(self.my_color))
                act = int(act)
                if 0 <= act and act <= 16:
                    act = IDX_TO_POS[act]
                    valid = True
                    return act
                else:
                    print('That is not a valid move! Please try again.')
            except Exception as e:
                print('{} is not a valid move! Please try again.'.format(act))
        return act

    def get_game_result(self, board, missed, last_move=None):
        if board.is_game_over() and board.winner() != self.my_color and board.winner() != DRAW:
            print('I lost....')

class PlayerQL:
    def __init__(self, color, name='QL', e=0.2, alpha=0.3):
        self.name = name
        self.my_color = color
        self.current_color = color
        self.q = {}
        self.e = e
        self.alpha = alpha
        self.gamma = 0.9
        self.last_move = None
        self.last_board = None
        self.total_game_count = 0
        self.random_ai = RandomAI()
        self.random_ai.debug(False)

    def act(self, board):
        return self.policy(board)

    def policy(self, board):
        self.last_board = PyBoard()
        self.last_board.copy(board)

        # explore sometimes
        if random.random() < (self.e / (self.total_game_count // 10000 + 1)):
            return self.random_ai.act(board)

        acts = board.acts()
        qs = [self.get_q(board.state(), act) for act in acts]
        # print([self.get_q(board.state(), act) for act in range(1, 17)])
        max_q = max(qs)

        if qs.count(max_q) > 1:
            # more than 1 best option; choose among them randomly
            best_options = [i for i in range(len(acts)) if qs[i] == max_q]
            i = random.choice(best_options)
        else:
            i = qs.index(max_q)

        self.last_move = acts[i]
        return IDX_TO_POS[acts[i]]

    def get_q(self, state, act):
        # encourage exploration; "optimistic" 1.0 initial values
        if self.q.get((state, act)) is None:
            self.q[(state, act)] = 1
        return self.q.get((state, act))

    def get_game_result(self, board, missed, last_move=None):
        if self.last_move is None:
            return

        if board.winner() == NONE:
            self.learn(self.last_board, self.last_move, 0, board)
            self.last_move = last_move
            self.last_board = board
        else:
            if board.winner() == self.current_color:
                self.learn(self.last_board, self.last_move, 1, board)
            elif board.winner() == self.current_color ^ 1:
                self.learn(self.last_board, self.last_move, -1, board)
            else:
                self.learn(self.last_board, self.last_move, 0, board)

            self.total_game_count += 1
            self.last_move = None
            self.last_board = None

    def learn(self, last_board, act, reward, board):
        state = last_board.state()
        f_state = board.state()
        p_q = self.get_q(state, act)
        if board.winner() != NONE or board.movable_pos == 0:
            max_q_new = 0
        else:
            max_q_new = max([self.get_q(f_state, a) for a in board.acts()])
        # print('{},{}'.format(self.q[(state, act)], max_q_new))
        # print('before:{}'.format(self.q[(f_state, act)] if (f_state, act) in self.q else None))
        # self.q[(f_state, act)] = p_q + self.alpha * ((reward + self.gamma * max_q_new) - p_q)
        q = p_q + self.alpha * ((reward + self.gamma * max_q_new) - p_q)
        self.q[(state, act)] = q
        # print('after:{}'.format(self.q[(f_state, act)]))
        logging.debug('{} with {} is updated from {} refs MAXQ={}:{}'.format(state, act, p_q, max_q_new, reward))
        logging.debug(self.q)

        for black, white, a in Board.rotate_position((*board.position, act)):
            b = PyBoard(black, white, board.current_color)
            self.q[(b.state(), a)] = q

if __name__ == '__main__':
    # コマンドライン引数の定義
    parser = argparse.ArgumentParser()
    parser.add_argument('--black', '-b', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'q'], help='black player')
    parser.add_argument('--white', '-w', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'q'], help='white player')
    parser.add_argument('--show_board', action='store_true', help='shows the boards')
    parser.add_argument('--show_result', action='store_true', help='shows the results')
    parser.add_argument('--stat', type=int, default=100, help='the dulation of showing stats')
    parser.add_argument('--num_play', type=int, default=10, help='the number of play')
    parser.add_argument('--log', type=str, default=None, help='log file path')
    parser.add_argument('--log_level', type=str, default='info', choices=['debug', 'info'], help='log level')
    args = parser.parse_args()

    # ログ出力の設定
    if args.log_level == 'debug':
        log_level = logging.DEBUG
    elif args.log_level == 'info':
        log_level = logging.INFO
    logging.basicConfig(format='%(asctime)s\t%(levelname)s\t%(message)s',
        datefmt='%Y/%m/%d %H:%M:%S', filename=args.log, level=log_level)

    # プレイヤーの設定
    if args.black == 'r':
        p1 = PlayerRandom(BLACK, 'Random Black')
    elif args.black == 'n':
        p1 = PlayerNegaMax(BLACK, 'NegaMax Black')
    elif args.black == 'mct':
        p1 = PlayerMonteCarloTree(BLACK, 'MonteCarloTree Black')
    elif args.black == 'h':
        p1 = PlayerHuman(BLACK, 'Human Black')
    elif args.black == 'q':
        p1 = PlayerQL(BLACK, 'Q Learning Black')

    if args.white == 'r':
        p2 = PlayerRandom(WHITE, 'Random White')
    elif args.white == 'n':
        p2 = PlayerNegaMax(WHITE, 'NegaMax White')
    elif args.white == 'mct':
        p2 = PlayerMonteCarloTree(WHITE, 'MonteCarloTree White')
    elif args.white == 'h':
        p2 = PlayerHuman(WHITE, 'Human White')
    elif args.white == 'q':
        p2 = PlayerQL(WHITE, 'Q Learning White')

    # 学習
    game = GameOrganizer(p1, p2, args.num_play, show_board=args.show_board, show_result=args.show_result, stat=args.stat)
    game.progress()
