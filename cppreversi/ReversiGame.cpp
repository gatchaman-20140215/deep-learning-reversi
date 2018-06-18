#include <iostream>
#include <string>
#include <memory>
#include <string.h>

#include "Reversi.h"
#include "Board.h"
#include "AI.h"

using namespace std;

int main(int argc, char* argv[]) {
	Board board;
    //RandomAI ai;
	MonteCarloAI ai1;
	NegaScoutAI ai2;

    std::uint64_t movables;
	std::uint64_t next_hand;

    do {
        movables = board.getMovablePos();
		cout << movables << endl;

        if (movables != 0) {
			if (board.getCurrentColor() == BLACK)
            	next_hand = ai1.act(board);
			else
				next_hand = ai2.act(board);
			board.move(next_hand);
        } else {
            board.pass();
        }

        cout << board.str() << endl;
    } while (!board.isGameOver());

    return 0;

}
