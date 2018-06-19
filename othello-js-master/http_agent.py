import json
from http.server import HTTPServer, SimpleHTTPRequestHandler
import urllib
import math
import argparse
import logging

from cpp_reversi import Board
from cpp_reversi import RandomAI, NegaMaxAI, NegaScoutAI, MonteCarloAI, MonteCarloTreeAI
from cpp_reversi import BLACK, WHITE

class Handler(SimpleHTTPRequestHandler):
    POINT_DICT = dict([(v, w) for (v, w) in zip(
        reversed([(1 << i) for i in range(64)]),
        [{'x': x, 'y': y} for y in range(8) for x in range(8)])])
    POINT_DICT[0] = {'x': -1, 'y': -1}

    def move(self, kyokumen):
        black, white = 0, 0
        for v in kyokumen['board']:
            if v == 'empty':
                black = black << 1
                white = white << 1
            elif v == 'black':
                black = (black << 1) | 1
                white = white << 1
            elif v == 'white':
                black = black << 1
                white = (white << 1) | 1
        logging.debug('black:{0:064b}'.format(black))
        logging.debug('white:{0:064b}'.format(white))
        current_color = BLACK if kyokumen['player'] == 'black' else WHITE

        board = Board(black, white, current_color)
        logging.info(board.to_string())
        logging.debug(kyokumen['moves'])

        if board.count_empty() > args.wld_depth:
            if kyokumen['ai'] == 'random':
                ai = RandomAI()
            elif kyokumen['ai'] == 'negamax':
                ai = NegaMaxAI(args.normal_depth, args.wld_depth, args.perfect_depth)
            elif kyokumen['ai'] == 'negascout':
                ai = NegaScoutAI(args.normal_depth, args.wld_depth, args.perfect_depth)
            elif kyokumen['ai'] == 'montecarlo':
                ai = MonteCarloAI(args.try_num)
            else:
                ai = MonteCarloTreeAI(args.try_num, args.exploration_param, args.playout_threshold)
            ai.debug(args.debug)
            best = ai.act(board)
        else:
            ai = NegaMaxAI(args.normal_depth, args.wld_depth, args.perfect_depth)
            ai.debug(args.debug)
            best = ai.act(board)

        logging.info('best:{}, moves:{}'.format(self.POINT_DICT[best], kyokumen['moves']))

        if best != 0:
            return kyokumen['moves'].index(self.POINT_DICT[best])
        else:
            return 0

        del board

    def do_POST(self):
        b = self.rfile.read(int(self.headers['Content-Length']))
        s = json.loads(b.decode('utf-8'))
        logging.debug(json.dumps(s, indent=4, sort_keys=True))

        best = self.move(s)
        ret = json.dumps(best).encode('UTF-8')

        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept')
        self.end_headers()
        self.wfile.write(ret)

if __name__ == '__main__':
    # 引数の定義
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', type=int, default=8080, help='port number')
    parser.add_argument('--normal_depth', '-d', type=int, default=9, help='normal search depth')
    parser.add_argument('--wld_depth', type=int, default=17, help='wld depth')
    parser.add_argument('--perfect_depth', type=int, default=15, help='perfect depth')
    parser.add_argument('--try_num', '-t', type=int, default=3000, help='MonteCarlo max try number')
    parser.add_argument('--exploration_param', type=float, default=math.sqrt(2), help='exploration param')
    parser.add_argument('--playout_threshold', type=int, default=10, help='playout threshold')
    parser.add_argument('--debug', action='store_true')
    parser.add_argument('--log', default=None, help='log file path')
    args = parser.parse_args()

    # ログ出力の設定
    logging.basicConfig(format='%(asctime)s\t%(levelname)s\t%(message)s',
        datefmt='%Y/%m/%d %H:%M:%S', filename=args.log, level=logging.INFO)

    httpd = HTTPServer(('', args.port), Handler)
    httpd.serve_forever()
