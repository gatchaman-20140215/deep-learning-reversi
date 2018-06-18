#include "Player.h"
#include "AI.h"

#include <complex>

Player::Player(const Color color, const std::string name) {
    this->color = color;
    this->name = name;
}

PlayerRandom::PlayerRandom(const Color color, const std::string name) : Player(color, name) {

}

void PlayerRandom::act(Board& board) {
    RandomAI ai;
    std::uint64_t hand = ai.act(board);
    board.move(hand);
}

PlayerNegaMax::PlayerNegaMax(const Color color, const std::string name) : Player(color, name) {

}

void PlayerNegaMax::act(Board& board) {
    NegaMaxAI ai;
    std::uint64_t hand = ai.act(board);
    board.move(hand);
}

PlayerNegaScout::PlayerNegaScout(const Color color, const std::string name) : Player(color, name) {

}

void PlayerNegaScout::act(Board& board) {
    NegaMaxAI ai;
    std::uint64_t hand = ai.act(board);
    board.move(hand);
}

PlayerMonteCarlo::PlayerMonteCarlo(const Color color, const std::string name) : Player(color, name) {
    this->trials = 3000;
}

PlayerMonteCarlo::PlayerMonteCarlo(const Color color, const std::string name, const int trials) : Player(color, name) {
    this->trials = trials;
}

void PlayerMonteCarlo::act(Board& board) {
    MonteCarloAI ai(this->trials);
    std::uint64_t hand = ai.act(board);
    board.move(hand);
}

PlayerMonteCarloTree::PlayerMonteCarloTree(const Color color, const std::string name) : Player(color, name) {
    this->trials = 3000;
    this->exploration_param = std::sqrt(2);
    this->playout_threshold = 10;
}

PlayerMonteCarloTree::PlayerMonteCarloTree(const Color color, const std::string name,
    const int trials, const double exploration_param, const int playout_threshold) : Player(color, name) {
    this->trials = trials;
    this->exploration_param = exploration_param;
    this->playout_threshold = playout_threshold;
}

void PlayerMonteCarloTree::act(Board& board) {
    MonteCarloTreeAI ai(this->trials, this->exploration_param, this->playout_threshold);
    std::uint64_t hand = ai.act(board);
    board.move(hand);
}
