#include "Reversi.h"
#include "Board.h"

class Player {
public:
    Player(const Color color, const std::string name);
    virtual void act(Board&) = 0;

protected:
    Color color;
    std::string name;
};

class PlayerRandom : public Player {
public:
    PlayerRandom(const Color color, const std::string name);
    void act(Board& board);
};

class PlayerNegaMax : public Player {
public:
    PlayerNegaMax(const Color color, const std::string name);
    void act(Board& board);
};

class PlayerNegaScout : public Player {
public:
    PlayerNegaScout(const Color color, const std::string name);
    void act(Board& board);
};

class PlayerMonteCarlo : public Player {
public:
    PlayerMonteCarlo(const Color color, const std::string name);
    PlayerMonteCarlo(const Color color, const std::string name, const int trials);
    void act(Board& board);

private:
    int trials;
};

class PlayerMonteCarloTree : public Player {
public:
    PlayerMonteCarloTree(const Color color, const std::string name);
    PlayerMonteCarloTree(const Color color, const std::string name,
        const int trials, const double exploration_param, const int playout_threshold);
    void act(Board& board);

private:
    int trials;
    double exploration_param;
    int playout_threshold;
};
