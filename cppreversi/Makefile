SRC = Util.cpp Board.cpp Evaluator.cpp MidEvaluator.cpp AI.cpp ReversiWrapper.cpp
OBJ = ReversiWrapper$(shell python3-config --extension-suffix)
#OBJ = ReversiWrapper.cpython-36m-x86_64-linux-gnu.so
CFLAGS = -O3 -Wall -shared -std=c++11 -fPIC $(shell python3 -m pybind11 --includes)
CC = g++
RM = rm -f

.PHONY: all
all: reversi

reversi: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ)

game: Util.cpp Board.cpp AI.cpp MidEvaluator.cpp ReversiGame.cpp
	$(CC) -Wall -O2 -fPIC -std=gnu++11 Util.cpp Board.cpp AI.cpp MidEvaluator.cpp ReversiGame.cpp \
		-o ReversiGame

.PHONY: clean
clean:
	$(RM) $(OBJ) ReversiGame
