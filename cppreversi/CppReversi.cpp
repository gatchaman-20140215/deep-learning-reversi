#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AI.h"
#include "Board.h"
#include "Evaluator.h"
#include "MidEvaluator.h"
#include "Player.h"
#include "Reversi.h"
#include "Util.h"

namespace py = pybind11;

PYBIND11_MODULE(cpp_reversi, m) {
    m.attr("BLACK") = &BLACK;
    m.attr("WHITE") = &WHITE;

    py::class_<Board> board(m, "Board");
    board
        .def(py::init<>())
        .def(py::init<std::uint16_t, std::uint16_t, Color>())
        .def_readonly("position", &Board::position)
        .def_readonly("turns", &Board::turns)
        .def_readonly("current_color", &Board::currentColor)
        .def_readonly("movable_pos", &Board::movablePos)
        .def("move", (bool (Board::*)(const std::uint16_t)) &Board::move)
        .def("skip", &Board::pass)
        .def("undo", &Board::undo)
        .def("is_game_over", &Board::isGameOver)
        .def("hash_key", &Board::generateHashKey)
        .def("count_disc", &Board::countDisc)
        //.def("movable_pos", &Board::getMovablePos)
        //.def("current_color", &Board::getCurrentColor)
        // .def("turns", &Board::getTurns)
        // .def("position", (std::uint16_t (Board::*)(const Color)) &Board::getPosition)
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

    py::class_<RandomAI> random_ai(m, "RandomAI");
    random_ai
        .def(py::init<>());

    py::class_<NegaMaxAI> negamax_ai(m, "NegaMaxAI");
    negamax_ai
        .def(py::init<>());

    py::class_<MonteCarloAI> montecarlo_ai(m, "MonteCarloAI");
    montecarlo_ai
        .def(py::init<>())
        .def(py::init<int>());

    py::class_<MonteCarloTreeAI> montecarlo_tree_ai(m, "MonteCarloTreeAI");
    montecarlo_tree_ai
        .def(py::init<>())
        .def(py::init<int, double, int>());

    py::class_<Evaluator> evaluator(m, "Evaluator");
    evaluator
        .def("evaluate", &Evaluator::evaluate)
        .def("get_class_name", &Evaluator::getClassName);

    py::class_<TableEvaluator>(m, "TableEvaluator", evaluator)
        .def(py::init<>());

    py::class_<WLDEvaluator>(m, "WLDEvaluator", evaluator)
        .def(py::init<>());

    py::class_<PerfectEvaluator>(m, "PerfectEvaluator", evaluator)
        .def(py::init<>());

    py::class_<PlayerRandom> player_random(m, "PlayerRandom");
    player_random
        .def(py::init<Color, std::string>())
        .def("act", &PlayerRandom::act);

    py::class_<PlayerNegaMax> player_negamax(m, "PlayerNegaMax");
    player_negamax
        .def(py::init<Color, std::string>())
        .def("act", &PlayerNegaMax::act);

    py::class_<PlayerNegaScout> player_negascout(m, "PlayerNegaScout");
    player_negascout
        .def(py::init<Color, std::string>())
        .def("act", &PlayerNegaScout::act);

    py::class_<PlayerMonteCarlo> player_montecarlo(m, "PlayerMonteCarlo");
    player_montecarlo
        .def(py::init<Color, std::string>())
        .def(py::init<Color, std::string, int>())
        .def("act", &PlayerMonteCarlo::act);

    py::class_<PlayerMonteCarloTree> player_montecarlo_tree(m, "PlayerMonteCarloTree");
    player_montecarlo_tree
        .def(py::init<Color, std::string>())
        .def(py::init<Color, std::string, int, double, int>())
        .def("act", &PlayerMonteCarloTree::act);

    m.doc() = "reversi4 plugin";


#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
