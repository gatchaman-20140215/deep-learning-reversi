#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <string>
#include "AI.h"
#include "Board.h"
#include "Evaluator.h"
#include "MidEvaluator.h"

namespace py = pybind11;

std::uint64_t montecarlo_ai(
        const std::uint64_t black, const std::uint64_t white, const Color currentColor, int trials, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
	   std::cout << board.str() << std::endl;

	MonteCarloAI* ai;
	ai = new MonteCarloAI(trials);
    ai->debug(debug);
	uint64_t best = ai->act(board);
	delete ai;
	ai = NULL;
	return best;
}

std::uint64_t montecarlo_tree_ai(const std::uint64_t black, const std::uint64_t white,
		const Color currentColor, int trials, double exploration_param, const int playout_threshold, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
	   std::cout << board.str() << std::endl;

	MonteCarloTreeAI* ai;
	ai = new MonteCarloTreeAI(trials, exploration_param, playout_threshold);
    ai->debug(debug);
	uint64_t best = ai->act(board);
	delete ai;
	ai = NULL;
	return best;
}

std::uint64_t negamax_ai(
        const std::uint64_t black, const std::uint64_t white, const Color currentColor, int depth, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
	   std::cout << board.str() << std::endl;

	NegaMaxAI* ai;
	ai = new NegaMaxAI(depth);
    ai->debug(debug);
	uint64_t best = ai->act(board);
	delete ai;
	ai = NULL;
	return best;
}

std::uint64_t negascout_ai(
        const std::uint64_t black, const std::uint64_t white, const Color currentColor, int depth, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
	   std::cout << board.str() << std::endl;

	NegaScoutAI* ai;
	ai = new NegaScoutAI(depth);
    ai->debug(debug);
	uint64_t best = ai->act(board);
	delete ai;
	ai = NULL;
	return best;
}

std::uint64_t random_ai(const std::uint64_t black, const std::uint64_t white, const Color currentColor, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
    	std::cout << board.str() << std::endl;

	RandomAI* ai;
	ai = new RandomAI();
    ai->debug(debug);
	uint64_t best = ai->act(board);
	delete ai;
	ai = NULL;
	return best;
}

int evaluate(const std::uint64_t black, const std::uint64_t white, const Color currentColor, bool debug) {
	Board board(black, white, currentColor);
    if (debug)
	   std::cout << board.str() << std::endl;

	PerfectEvaluator evaluator;
	int score = evaluator.evaluate(board);
	return score;
}

PYBIND11_MODULE(ReversiWrapper, m) {
    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def(py::init<std::uint64_t, std::uint64_t, Color>())
        .def_readonly("position", &Board::position)
        .def_readonly("turns", &Board::turns)
        .def_readonly("current_color", &Board::currentColor)
        .def_readonly("movable_pos", &Board::movablePos)
        .def("move", (bool (Board::*)(const std::uint64_t)) &Board::move)
        .def("skip", &Board::pass)
        .def("undo", &Board::undo)
        .def("is_game_over", &Board::isGameOver)
        .def("hash_key", &Board::generateHashKey)
        .def("count_disc", &Board::countDisc)
        // .def("movable_pos", &Board::getMovablePos)
        // .def("current_color", &Board::getCurrentColor)
        // .def("turns", &Board::getTurns)
        // .def("position", (std::uint64_t (Board::*)(const Color)) &Board::getPosition)
        .def("empty", &Board::getEmpty)
        .def("count_empty", &Board::countEmpty)
        .def("to_string", &Board::str)
        .def("winner", &Board::winner)
        .def("copy", &Board::copy)
        .def_static("rotate_horizontally", &Board::rotateHorizontally)
        .def_static("rotate_vertically", &Board::rotateVertically)
        .def_static("transpose", &Board::transpose)
        .def_static("rotate_position", &Board::rotatePosition);

    py::class_<AI> ai(m, "AI");
    ai
        .def("act", &AI::act)
        .def("get_class_name", &AI::getClassName)
        .def("debug", &AI::debug);

    py::class_<RandomAI>(m, "RandomAI", ai)
        .def(py::init<>());

    py::class_<NegaMaxAI>(m, "NegaMaxAI", ai)
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<int, int, int>());

    py::class_<NegaScoutAI>(m, "NegaScoutAI", ai)
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<int, int, int>());

    py::class_<MonteCarloAI>(m, "MonteCarloAI", ai)
        .def(py::init<>())
        .def(py::init<int>());

    py::class_<MonteCarloTreeAI>(m, "MonteCarloTreeAI", ai)
        .def(py::init<>())
        .def(py::init<int, double, int>());


    py::class_<Evaluator> evaluator(m, "Evaluator");
    evaluator
        .def("evaluate", &Evaluator::evaluate)
        .def("get_class_name", &Evaluator::getClassName);

    py::class_<TableEvaluator>(m, "TableEvaluator", evaluator)
        .def(py::init<>());

    py::class_<MidEvaluator>(m, "MidEvaluator", evaluator)
        .def(py::init<>());

    py::class_<WLDEvaluator>(m, "WLDEvaluator", evaluator)
        .def(py::init<>());

    py::class_<PerfectEvaluator>(m, "PerfectEvaluator", evaluator)
        .def(py::init<>());

    m.doc() = "reversi plugin";
    m.def("montecarlo_ai", &montecarlo_ai,
        py::arg("black"), py::arg("white"), py::arg("current_color"), py::arg("trials"), py::arg("debug"));
	m.def("montecarlo_tree_ai", &montecarlo_tree_ai,
        py::arg("black"), py::arg("white"), py::arg("current_color"),
        py::arg("trials"), py::arg("exploration_param"), py::arg("playout_threshold"), py::arg("debug"));
	m.def("negamax_ai", &negamax_ai,
        py::arg("black"), py::arg("white"), py::arg("current_color"), py::arg("depth"), py::arg("debug"));
    m.def("negascout_ai", &negamax_ai,
        py::arg("black"), py::arg("white"), py::arg("current_color"), py::arg("depth"), py::arg("debug"));
	m.def("random_ai", &random_ai, py::arg("black"), py::arg("white"), py::arg("current_color"), py::arg("debug"));
	m.def("evaluate", &evaluate, py::arg("black"), py::arg("white"), py::arg("current_color"), py::arg("debug"));
}
