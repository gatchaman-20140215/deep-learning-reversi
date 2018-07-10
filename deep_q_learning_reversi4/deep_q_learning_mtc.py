from cpp_reversi4 import Board, RandomAI, NegaMaxAI, MonteCarloTreeAI
from cpp_reversi4 import BLACK, WHITE, DRAW, NONE

import random
import math

import argparse
import logging
from time import sleep
import time

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

    def legal_moves(self):
        if self.movable_pos == 0:
            return []
        else:
            a = []
            mask = 0x8000
            for i in range(16):
                if (self.movable_pos & mask) != 0:
                    a.append(i)
                mask = mask >> 1
            return a

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

            self.players.reverse()

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

class NN(chainer.Chain):
    def __init__(self):
        super(NN, self).__init__()
        with self.init_scope():
            self.l1 = L.Linear(33, 96)
            self.l2 = L.Linear(96, 96)
            # policy network
            self.l3 = L.Linear(96, 16)
            # value network
            self.l3_v = L.Linear(96, 1)

    def __call__(self, x, t1=None, t2=None, train=False):
        h1 = F.leaky_relu(self.l1(x))
        h2 = F.leaky_relu(self.l2(h1))
        # policy network
        h3 = F.leaky_relu(self.l3(h2))
        # value network
        h3_v = F.leaky_relu(self.l3_v(h2))

        if train:
            # return F.mean_squared_error(h3, t1) + F.sigmoid_cross_entropy(h3_v, t2)
            return F.mean_squared_error(h3, t1) + F.mean_squared_error(h3_v, t2)
        else:
            return h3, h3_v

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
        h1 = F.max_pooling_2d(F.leaky_relu(self.cn1(x)), 2)
        h2 = F.max_pooling_2d(F.leaky_relu(self.cn2(h1)), 2)
        h3 = F.max_pooling_2d(F.leaky_relu(self.cn3(h2)), 2)
        h4 = F.leaky_relu(self.l1(h3))
        h5 = self.l2(h4)

        if train:
            return F.mean_squared_error(h5, t)
        else:
            return h5

# UCBのボーナス項の定数
C_PUCT = 1.0
# 1手当たりのプレイアウト数
CONST_PLAYOUT = 300
# 温度パラメータ
TEMPERATURE = 1.0

def softmax_temperature_with_normalize(logits, temperature):
    # 温度パラメータを適用
    logits /= temperature

    # 勝率を計算（オーバーフローを防止するため最大値で引く）
    max_logit = max(logits)
    probabilities = np.exp(logits - max_logit)

    # 合計が1になるよう正規化
    sum_probabilities = sum(probabilities)
    probabilities /= sum_probabilities

    return probabilities

class PlayoutInfo:
    def __init__(self):
        self.halt = 0   # 探索を打ち切る回数
        self.count = 0  # 現在の探索回数

class PlayerMTCS:
    def __init__(self, color, name='MTCS', e=1, disp_pred=False):
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

        # モデルファイルのパス
        # self.model_file = './model_policy_value'
        # self.model = None
        # ノード情報
        self.node_hash = NodeHash()
        self.node_hash.initialize()
        self.uct_node = [UctNode() for _ in range(UCT_HASH_SIZE)]
        # プレイアウト回数管理
        self.po_info = PlayoutInfo()
        self.playout = CONST_PLAYOUT
        # 温度パラメータ
        self.temperature = TEMPERATURE

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

        # 探索情報をクリア
        self.po_info.count = 0

        # 古いハッシュを削除
        self.node_hash.delete_old_hash(board, self.uct_node)

        # 探索開始時刻の記録
        begin_time = time.time()

        # 探索回数の閾値を設定
        self.po_info.halt = self.playout

        # ルートノードの展開
        self.current_root = self.expand_node(board)

        # 候補手が1つの場合は、その手を返す。
        current_node = self.uct_node[self.current_root]
        child_num = current_node.child_num
        child_move = current_node.child_move
        if child_num == 1:
            logging.debug('bestmove:{}'.format(child_move[0]))
            return IDX_TO_POS[child_move[0]]

        # プレイアウトを繰り返す。
        # 探索回数が閾値を超える、または探索が打ち切られたらループを抜ける。
        while self.po_info.count < self.po_info.halt:
            # 探索回数を1回増やす。
            self.po_info.count += 1
            # 1回プレイアウトする。
            self.uct_search(board, self.current_root)
            # 探索を打ち切るか、確認
            if self.interruption_check() or not self.node_hash.enough_size:
                break

        # 探索にかかった時間を求める。
        finish_time = time.time() - begin_time

        child_move_count = current_node.child_move_count
        if board.turns < 4:
            # 訪問回数に応じた確率で手を選択する。
            selected_index = np.random.choice(np.arange(child_num), p=child_move_count/sum(child_move_count))
        else:
            # 訪問回数最大のてを選択する。
            selected_index = np.argmax(child_move_count)

        child_win = current_node.child_win

        if logging.getLogger().isEnabledFor(logging.DEBUG):
            for i in range(child_num):
                logger.debug('{:3}:{:5} move_count:{:4} nn_rage:{:.5f} win_rate:{:5f}'.format(
                    i, child_move[i], child_move_count[i], current_node.nnrate[i],
                    child_win[i] / child_move_count[i] if child_move_count[i] > 0 else 0))

        # 選択した着手の勝率を計算
        best_wp = child_win[selected_index] / child_move_count[selected_index]

        best_move = child_move[selected_index]

        # 勝率を評価値に変換
        if best_wp == 1.0:
            cp = 30000
        else:
            cp = int(-math.log(1.0 / best_wp - 1.0) * 600)

        logging.debug('nps:{}, time:{}, nodes:{}, hashfull:{}, score cp:{}, pv:{}'.format(
            int(current_node.move_count / finish_time),
            int(finish_time * 1000),
            current_node.move_count,
            int(self.node_hash.get_usage_rate() * 1000),
            cp, best_move))

        return IDX_TO_POS[best_move]

        # x = np.array([board.convert_nn()], dtype=np.float32).astype(np.float32)
        # pred = self.model(x)
        # # logging.debug(pred.data)
        # # self.last_pred = pred.data[0, :]
        # act = np.argmax(pred[0].data, axis=1)[0]
        # i = 0
        # while (IDX_TO_POS[act] & board.movable_pos) == 0:
        #     self.learn(self.last_board, act, self.reward_miss, self.last_board)
        #     pred = self.model(x)
        #     logging.debug(pred[0].data)
        #     act = np.argmax(pred[0].data, axis=1)[0]
        #     i += 1
        #     if i > 10:
        #         logging.debug('Exceed Pos Fild {} with {}'.format(board.to_string(), act))
        #         act = POS_TO_IDX[self.random_ai.act(board)]
        #
        # self.last_move = act
        # return IDX_TO_POS[act]

    # UCB値が最大の手を求める。
    def select_max_ucb_child(self, board, current_node):
        child_num = current_node.child_num
        child_win = current_node.child_win
        child_move_count = current_node.child_move_count

        # print('child_win:{}, child_move_count:{}'.format(child_win, child_move_count))
        # print(np.repeat(np.float32(0.5), child_num))
        # q = 0
        q = np.divide(child_win, child_move_count, out=np.repeat(np.float32(0.5), child_num), where=child_move_count!=0)
        u = np.sqrt(np.float32(current_node.move_count)) / (1 + child_move_count)
        ucb = q + C_PUCT * current_node.nnrate * u

        return np.argmax(ucb)

    # ノードの展開
    def expand_node(self, board):
        index = self.node_hash.find_same_hash_index(board.state(), board.current_color, board.turns)

        # 合流先が検知できれば、それを返す。
        if not index == UCT_HASH_SIZE:
            return index

        # 空のインデックスを探す。
        index = self.node_hash.search_empty_index(board.state(), board.current_color, board.turns)

        # 現在のノードの初期化
        current_node = self.uct_node[index]
        current_node.move_count = 0
        current_node.win = 0
        current_node.child_num = 0
        current_node.evaled = False
        current_node.value_win = 0.0

        # 候補手の展開
        current_node.child_move = [move for move in board.legal_moves()]
        child_num = len(current_node.child_move)
        current_node.child_index = [NOT_EXPANDED for _ in range(child_num)]
        current_node.child_move_count = np.zeros(child_num, dtype=np.float32)
        current_node.child_win = np.zeros(child_num, dtype=np.float32)

        # 子ノードの個数を設定
        current_node.child_num = child_num

        # ノードを評価
        if child_num > 0:
            self.eval_node(board, index)
        else:
            current_node.value_win = 0.0
            current_node.evaled = True

        return index

    # 探索を打ち切るか確認
    def interruption_check(self):
        child_num = self.uct_node[self.current_root].child_num
        child_move_count = self.uct_node[self.current_root].child_move_count
        rest = self.po_info.halt - self.po_info.count

        # 探索回数が最も多い手と次に多い手を求める。
        second, first = child_move_count[np.argpartition(child_move_count, -2)[-2:]]

        # 残りの探索を全て次善手に費やしても最善手を越えられない場合は、探索を打ち切る。
        if first - second > rest:
            return True
        else:
            return False

    # UCT探索
    def uct_search(self, board, current):
        current_node = self.uct_node[current]

        # 詰みのチェック
        if current_node.child_num == 0:
            return 1.0      # 反転して値を返すため1を返す。

        child_move = current_node.child_move
        child_move_count = current_node.child_move_count
        child_index = current_node.child_index

        # UCB値が最大の手を求める。
        next_index = self.select_max_ucb_child(board, current_node)
        # 選んだ手を着手
        board.move(IDX_TO_POS[child_move[next_index]])

        # ノードの展開の確認
        if child_index[next_index] == NOT_EXPANDED:
            # ノードの展開（ノード展開処理の中でノードを評価する）
            index = self.expand_node(board)
            child_index[next_index] = index
            child_node = self.uct_node[index]

            # valueを勝敗として返す。
            result = 1 - child_node.value_win
        else:
            # 手番を入れ替えて1手深く読む。
            result = self.uct_search(board, child_index[next_index])

        # 探索結果の反映
        current_node.win += result
        current_node.move_count += 1
        current_node.child_win[next_index] += result
        current_node.child_move_count[next_index] += 1

        # 手を戻す。
        board.undo()

        return 1 - result

    # ノードを評価
    def eval_node(self, board, index):
        x = np.array([board.convert_nn()], dtype=np.float32).astype(np.float32)
        y1, y2 = self.model(x)
        logits = y1.data[0]
        value = F.sigmoid(y2).data[0]

        current_node = self.uct_node[index]
        child_num = current_node.child_num
        child_move = current_node.child_move
        color = self.node_hash[index].color

        # 合法手でフィルター
        legal_move_labels = []
        for i in range(child_num):
            legal_move_labels.append(child_move[i] - 1)

        # Boltzmann分布
        probabilities = softmax_temperature_with_normalize(logits[legal_move_labels], self.temperature)

        # ノードの値を更新
        current_node.nnrate = probabilities
        current_node.value_win = float(value)
        current_node.evaled = True

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
            max_q_new = (0, 0)
        else:
            x = np.array([board.convert_nn()], dtype=np.float32).astype(np.float32)
            prediction = self.model(x)
            max_q_new = (np.max(prediction[0].data[0]), prediction[1].data[0][0])
        # update = (reward + self.gamma * max_q_new[0], round(reward + self.gamma * max_q_new[1]))
        update = (reward + self.gamma * max_q_new[0], reward + self.gamma * max_q_new[1])
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

        pred = self.model(x)[0].data
        for i, a in enumerate(acts):
            pred[i][a] = update[0]

        t1 = np.array(pred, dtype=np.float32).astype(np.float32)
        t2 = np.array([[update[1]]], dtype=np.float32).astype(np.float32)
        self.model.zerograds()
        loss = self.model(x, t1, t2, train=True)
        loss.backward()
        self.optimizer.update()

class UctNode:
    def __init__(self):
        self.move_count = 0         # ノードの訪問回数
        self.win = 0.0              # 勝率の合計
        self.child_num = 0          # 子ノードの数
        self.child_act = None       # 子ノードの指し手
        self.child_index = None     # 子ノードのインデックス
        self.nnrate = None          # 方策ネットワークの予測確率
        self.value_win = 0.0        # 価値ネットワークの予測勝率
        self.evaled = False         # 評価済みフラグ

UCT_HASH_SIZE = 4096    # 2のn乗であること
UCT_HASH_LIMIT = UCT_HASH_SIZE * 9 / 10

# 未展開のノードのインデックス
NOT_EXPANDED = -1

def hash_to_index(hash):
    return (hash & 0xffffffff) & (UCT_HASH_SIZE - 1)

class NodeHashEntry:
    def __init__(self):
        self.hash = 0               # ハッシュ値
        self.color = 0              # 手番
        self.moves = 0              # ゲーム開始からの手数
        self.flag = False           # 使用中か識別するフラグ

class NodeHash:
    def __init__(self):
        self.used = 0
        self.enough_size = True
        self.node_hash = None

    # UCTノードのハッシュの初期化
    def initialize(self):
        self.used = 0
        self.enough_size = True

        if self.node_hash is None:
            self.node_hash = [NodeHashEntry() for _ in range(UCT_HASH_SIZE)]
        else:
            for i in range(UCT_HASH_SIZE):
                self.node_hash[i].hash = 0
                self.node_hash[i].color = 0
                self.node_hash[i].moves = 0
                self.node_hash[i].flag = False

    # 配列の添え字でノードを取得する。
    def __getitem__(self, i):
        return self.node_hash[i]

    # 未使用のインデックスを探して返す。
    def search_empty_index(self, hash, color, moves):
        key = hash_to_index(hash)
        i = key

        while True:
            if not self.node_hash[i].flag:
                self.node_hash[i].hash = hash & 0xffffffff
                # self.node_hash[i].color = (hash & 0x100000000) >>  32
                self.node_hash[i].color = color
                self.node_hash[i].moves = moves
                self.node_hash[i].flag = True
                self.used += 1
                if self.used > UCT_HASH_LIMIT:
                    self.enough_size = False
                return i
            i += 1
            if i >= UCT_HASH_SIZE:
                i = 0
            if i == key:
                return UCT_HASH_SIZE

    # ハッシュ値に対応するインデックスを返す。
    def find_same_hash_index(self, hash, color, moves):
        key = hash_to_index(hash)
        i = key

        while True:
            if not self.node_hash[i].flag:
                return UCT_HASH_SIZE
            elif self.node_hash[i].hash == hash and self.node_hash[i].color == color and self.node_hash[i].moves == moves:
                return i
            i += 1
            if i >= UCT_HASH_SIZE:
                i = 0
                if i == key:
                    return UCT_HASH_SIZE

    # 使用中のノードを残す。
    def save_used_hash(self, board, uct_node, index):
        self.node_hash[index].flag = True
        self.used += 1

        current_node = uct_node[index]
        child_index = current_node.child_index
        child_move = current_node.child_move
        child_num = current_node.child_num
        for i in range(child_num):
            if child_index[i] != NOT_EXPANDED and self.node_hash[child_index[i]].flag == False:
                board.move(IDX_TO_POS[child_move[i]])
                self.save_used_hash(board, uct_node, child_index[i])
                board.undo()

    # 古いハッシュを削除
    def delete_old_hash(self, board, uct_node):
        # 現在の局面をルートとする局面以外を削除する。
        root = self.find_same_hash_index(board.state(), board.current_color, board.turns)

        self.used = 0
        for i in range(UCT_HASH_SIZE):
            self.node_hash[i].flag = False

        if root != UCT_HASH_SIZE:
            self.save_used_hash(board, uct_node, root)

        self.enough_size = True

    def get_usage_rate(self):
        return self.used / UCT_HASH_SIZE

if __name__ == '__main__':
    # コマンドライン引数の定義
    parser = argparse.ArgumentParser()
    parser.add_argument('--black', '-b', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'mtcs'], help='black player')
    parser.add_argument('--white', '-w', type=str, default='r', choices=['r', 'n', 'mct', 'h', 'mtcs'], help='white player')
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
    elif args.black == 'mtcs':
        p1 = PlayerMTCS(BLACK, 'deep Q Network Black')

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
        p2 = PlayerDQN(WHITE, 'deep Q Network White')
    elif args.white == 'mtcs':
        p2 = PlayerMTCS(WHITE, 'deep Q Network White')

    # 学習
    game = GameOrganizer(p1, p2, args.num_play, show_board=args.show_board, show_result=args.show_result, stat=args.stat)
    game.progress()
