# -*- coding: utf-8 -*-

import chainer
import chainer.functions as F
import chainer.links as L
import chainerrl
import numpy as np
import math

from cpp_reversi4 import Board, RandomAI, MonteCarloTreeAI, BLACK, WHITE, DRAW

import argparse
import logging

mask = 0x8000
IDX_TO_POS = {0: 0}
POS_TO_IDX = {0: 0}
for i in range(16):
    IDX_TO_POS[i + 1] = mask
    POS_TO_IDX[mask] = i + 1
    mask = mask >> 1

# explorer用ランダム関数オブジェクト
class RandomActor:
    def __init__(self, board):
        self.ai = RandomAI()
        self.ai.debug(False)
        self.board = board
        self.random_count = 0

    def random_action_func(self):
        self.random_count += 1
        return POS_TO_IDX[self.ai.act(self.board)]

class MonteCarloTreeActor:
    def __init__(self, board):
        self.ai = MonteCarloTreeAI(1000, math.sqrt(2), 10)
        self.ai.debug(False)
        self.board = board
        self.random_count = 0

    def random_action_func(self):
        self.random_count += 1
        return POS_TO_IDX[self.ai.act(self.board)]

# Q関数
class QFunction(chainer.Chain):
    def __init__(self):
        super(QFunction, self).__init__()
        with self.init_scope():
            self.cn1 = L.Convolution2D(in_channels=5, out_channels=32, ksize=3, pad=1)
            self.cn2 = L.Convolution2D(in_channels=32, out_channels=32, ksize=3, pad=1)
            self.cn3 = L.Convolution2D(in_channels=32, out_channels=32, ksize=3, pad=1)
            self.l1 = L.Linear(32, 96)
            self.l2 = L.Linear(96, 17)

    def __call__(self, x, t=False):
        x = x.astype('float32')
        return chainerrl.action_value.DiscreteActionValue(self.fwd(x))

    def fwd(self, x):
        h1 = F.max_pooling_2d(F.relu(self.cn1(x)), 2)
        h2 = F.max_pooling_2d(F.relu(self.cn2(h1)), 2)
        h3 = F.max_pooling_2d(F.relu(self.cn3(h2)), 2)
        h4 = F.relu(self.l1(h3))
        return self.l2(h4)

# コマンドライン引数の定義
parser = argparse.ArgumentParser()
parser.add_argument('--eps', type=float, default=0.0001)
parser.add_argument('--gamma', type=float, default=0.95)
parser.add_argument('--decay_steps', type=int, default=50000)
parser.add_argument('--n_episodes', type=int, default=20000)
parser.add_argument('--gpu', action='store_true')
parser.add_argument('--log', default=None, help='log file path')
args = parser.parse_args()

# ログ出力の設定
logging.basicConfig(format='%(asctime)s\t%(levelname)s\t%(message)s',
    datefmt='%Y/%m/%d %H:%M:%S', filename=args.log, level=logging.INFO)

if args.gpu:
    import cupy as cp
else:
    cp = np

# 盤面
board = Board()
# explorer用ランダム関数オブジェクト
random_actor = MonteCarloTreeActor(board)
# Q関数とオプティマイザの設定
q_func = QFunction()
if args.gpu:
    chainer.cuda.get_device_from_id(0).use()
    chainer.cuda.check_cuda_available()
    q_func.to_gpu
optimizer = chainer.optimizers.Adam(eps=args.eps)
optimizer.setup(q_func)
# 報酬の割引率
gamma = args.gamma
# Epsilon-greedyを使ってたまに冒険、50000ステップでend_epsilonとなる。
explorer = chainerrl.explorers.LinearDecayEpsilonGreedy(
    start_epsilon=1.0, end_epsilon=0.3, decay_steps=args.decay_steps, random_action_func=random_actor.random_action_func)
# Experience ReplayというDQNで用いる学習手法で使うバッファ
replay_buffer = chainerrl.replay_buffer.ReplayBuffer(capacity=10**12)
# Agentの生成（replay_buffer等を共有する2つ）
agent_p1 = chainerrl.agents.DoubleDQN(
    q_func, optimizer, replay_buffer, gamma, explorer,
    replay_start_size=500, update_interval=1, target_update_interval=100
)
agent_p2 = chainerrl.agents.DoubleDQN(
    q_func, optimizer, replay_buffer, gamma, explorer,
    replay_start_size=500, update_interval=1, target_update_interval=100
)


# カウンタの宣言
miss = 0
win = 0
draw = 0
# エピソードの繰り返し実行
for i in range(1, args.n_episodes + 1):
    board.init()
    reward = 0
    agents = [agent_p1, agent_p2]
    last_state = None
    missed = False

    turn = np.random.choice([0, 1])
    # print(board.to_string())
    while True:
        hand = agents[turn].act_and_train(cp.array(board.convert_nn(), dtype='float32'), reward)
        # print('hand:{}'.format(hand))
        result = board.move(IDX_TO_POS[hand])

        if result == False:
            miss += 1
            missed = True

        # print(board.to_string())

        if board.is_game_over() or missed is True:
            if board.winner() == board.current_color:
                reward = 1
                win += 1
            elif board.winner() == DRAW:
                draw += 1
                reward = 0.5
            else:
                reward = 0
            if missed is True:
                miss += 1

            # エピソードを終了して学習
            agents[turn].stop_episode_and_train(cp.array(board.convert_nn(), dtype='float32'), reward, True)
            # 相手もエピソードを終了して学習。相手のミスは勝利として学習しない。
            if agents[turn ^ 1].last_state is not None and missed is False:
                # 前のターンで取っておいたlast_stateをaction実行後の状態として渡す。
                agents[turn ^ 1].stop_episode_and_train(last_state, reward * -1, True)

            break
        else:
            # 学習用にターン最後の状態を退避
            last_state = board.convert_nn()
            # ターンを切替
            turn ^= 1

    # コンソールに進捗表示
    if i % 100 == 0:
        logging.info('episode:{} / rnd:{} / miss:{} / win:{} / draw:{} / statistics:{} / epsilon:{}'.format(
            i, random_actor.random_count, miss, win, draw, agent_p1.get_statistics(), agent_p1.explorer.epsilon))
        # カウンタの初期化
        miss = 0
        win = 0
        draw = 0
        random_actor.random_count = 0
    if i % 10000 == 0:
        # 10000エピソードごとにモデルを保存
        agent_p1.save('result_{}'.format(i))

print('Training finished.')
