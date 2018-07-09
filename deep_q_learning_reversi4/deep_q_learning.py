from cpp_reversi4 import Board, RandomAI, NegaMaxAI, MonteCarloTreeAI
from cpp_reversi4 import BLACK, WHITE, DRAW, NONE

import random
import math

import argparse
import logging
from time import sleep

import chainer
from chainer import Function, gradient_check, Variable, optimizers, serializers, utils
import chainer.functions as F
import chainer.links as L
from chainer import computational_graph as c
import numpy as np

mask = 0x8000
IDX_TO_POS = {}
POS_TO_IDX = {}
for i in range(16):
    IDX_TO_POS[i] = mask
    POS_TO_IDX[mask] = i
    mask = mask >> 1

class PyBoard(Board):
    def state(self):
        # return (self.current_color << 32) + (self.position[BLACK] << 16) + self.position[WHITE]
        # return (self.position[self.current_color] << 16) + self.position[self.current_color ^ 1]
        # return (self.position[BLACK] << 16) | self.position[WHITE]
        return ((self.current_color << 32) | self.position[BLACK] << 16) | self.position[WHITE]

    def convert_nn(self):
        if args.network == 'nn':
            return list('{:033b}'.format(self.state()))
        else:
            return super().convert_nn()

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
                    self.players[turn].get_game_result(board, missed=False)

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
        if board.movable_pos == 0:
            return 0

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

    def get_game_result(self, board, missed):
        if self.last_move is None:
            return

        if board.winner() == NONE:
            self.learn(self.last_board, self.last_move, 0, board)
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

class NN(chainer.Chain):
    def __init__(self):
        super(NN, self).__init__()
        with self.init_scope():
            self.l1 = L.Linear(33, 96)
            self.l2 = L.Linear(96, 96)
            self.l3 = L.Linear(96, 16)

    def __call__(self, x, t=None, train=False):
        h1 = F.leaky_relu(self.l1(x))
        h2 = F.leaky_relu(self.l2(h1))
        h3 = F.leaky_relu(self.l3(h2))

        if train:
            return F.mean_squared_error(h3, t)
        else:
            return h3

class CNN(chainer.Chain):
    def __init__(self):
        super(CNN, self).__init__()
        with self.init_scope():
            self.cn1 = L.Convolution2D(in_channels=5, out_channels=32, ksize=3, pad=1)
            self.cn2 = L.Convolution2D(in_channels=32, out_channels=32, ksize=3, pad=1)
            self.cn3 = L.Convolution2D(in_channels=32, out_channels=32, ksize=3, pad=1)
            self.l1 = L.Linear(32, 96)
            self.l2 = L.Linear(96, 16)

    def __call__(self, x, t=None, train=False):
        h1 = F.max_pooling_2d(F.relu(self.cn1(x)), 2)
        h2 = F.max_pooling_2d(F.relu(self.cn2(h1)), 2)
        h3 = F.max_pooling_2d(F.relu(self.cn3(h2)), 2)
        h4 = F.relu(self.l1(h3))
        h5 = self.l2(h4)

        if train:
            return F.mean_squared_error(h5, t)
        else:
            return h5

class PlayerDQN:
    def __init__(self, color, name='DQN', e=1, disp_pred=False):
        self.name = name
        self.my_color = color
        self.current_color = color
        if args.network == 'nn':
            self.model = NN()
        else:
            self.model = CNN()
        self.optimizer = optimizers.SGD()
        self.optimizer.setup(self.model)
        self.e = e
        self.gamma = 0.95
        self.disp_pred = disp_pred
        self.last_move = None
        self.last_board = None
        # self.last_pred = None
        self.total_game_count = 0
        self.reward_win, self.reward_lose, self.reward_draw, self.reward_miss = 1, -1, 0, -1.5
        self.random_ai = RandomAI()
        self.random_ai.debug(False)

    def act(self, board):
        if board.movable_pos == 0:
            return 0

        self.last_board = PyBoard()
        self.last_board.copy(board)

        if self.e > 0.2:    # decrement epsilon over time
            self.e -= 1 / 20000
        if random.random() < self.e:
            act = self.random_ai.act(board)
            self.last_move = POS_TO_IDX[act]
            return act

        x = np.array([board.convert_nn()], dtype=np.float32).astype(np.float32)
        pred = self.model(x)
        # logging.debug(pred.data)
        # self.last_pred = pred.data[0, :]
        act = np.argmax(pred.data, axis=1)[0]
        i = 0
        while (IDX_TO_POS[act] & board.movable_pos) == 0:
            self.learn(self.last_board, act, self.reward_miss, self.last_board)
            pred = self.model(x)
            logging.debug(pred.data)
            act = np.argmax(pred.data, axis=1)[0]
            i += 1
            if i > 10:
                logging.debug('Exceed Pos Fild {} with {}'.format(board.to_string(), act))
                act = POS_TO_IDX[self.random_ai.act(board)]

        self.last_move = act
        return IDX_TO_POS[act]

    def get_game_result(self, board, missed):
        if self.last_move is None:
            return

        if missed:
            self.learn(self.last_board, self.last_move, self.reward_miss, board)

            self.total_game_count += 1
            self.last_move = None
            self.last_board = None
            # self.last_pred = None
            return

        if board.winner() == NONE:
            self.learn(self.last_board, self.last_move, 0, board)
            # self.last_move = last_move
        else:
            if board.winner() == self.current_color:
                self.learn(self.last_board, self.last_move, self.reward_win, board)
            elif board.winner() == self.current_color ^ 1:
                self.learn(self.last_board, self.last_move, self.reward_lose, board)
            else:
                self.learn(self.last_board, self.last_move, self.reward_draw, board)

            self.total_game_count += 1
            self.last_move = None
            self.last_board = None
            # self.last_pred = None

    def learn(self, last_board, act, reward, board):
        if board.winner() != NONE or board.movable_pos == 0:
            max_q_new = 0
        else:
            x = np.array([board.convert_nn()], dtype=np.float32).astype(np.float32)
            max_q_new = np.max(self.model(x).data[0])
        update = reward + self.gamma * max_q_new
        logging.debug('Prev Board:{}, act:{}, Next Board:{}, Get Reward:{}, Update:{}'.format(
            last_board.to_string(), act, board.to_string(), reward, update))
        # logging.debug('prev:{}'.format(self.last_pred))
        # self.last_pred[act] = update

        acts = [act]
        x = np.array([last_board.convert_nn()], dtype=np.float32).astype(np.float32)
        # for black, white, a in Board.rotate_position((*last_board.position, IDX_TO_POS[act])):
        #     b = PyBoard(black, white, last_board.current_color)
        #     x = np.append(x, np.array([b.convert_nn()], dtype=np.float32).astype(np.float32), axis=0)
        #     acts.append(POS_TO_IDX[a])

        pred = self.model(x).data
        for i, a in enumerate(acts):
            pred[i][a] = update

        t = np.array(pred, dtype=np.float32).astype(np.float32)
        self.model.zerograds()
        loss = self.model(x, t, train=True)
        loss.backward()
        self.optimizer.update()
        # logging.debug('Updated:{}'.format(self.model(x).data))
        # logging.debug('{} with {} is updated from {} refs MAXQ={}:{}'.format(board.to_string(), act, p_q, max_q_new, reward))
        # logging.debug(self.q)


if __name__ == '__main__':
    # コマンドライン引数の定義
    parser = argparse.ArgumentParser()
    parser.add_argument('--black', '-b', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'q', 'dq'], help='black player')
    parser.add_argument('--white', '-w', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'q', 'dq'], help='white player')
    parser.add_argument('--show_board', action='store_true', help='shows the boards')
    parser.add_argument('--show_result', action='store_true', help='shows the results')
    parser.add_argument('--stat', type=int, default=100, help='the dulation of showing stats')
    parser.add_argument('--num_play', type=int, default=10, help='the number of play')
    parser.add_argument('--network', type=str, default='nn', choices=['nn', 'cnn'], help='neural network type')
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
    elif args.black == 'dq':
        p1 = PlayerDQN(BLACK, 'deep Q Network Black')

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
    elif args.white == 'dq':
        p2 = PlayerDQN(WHITE, 'deep Q Network Black')

    # 学習
    game = GameOrganizer(p1, p2, args.num_play, show_board=args.show_board, show_result=args.show_result, stat=args.stat)
    game.progress()
