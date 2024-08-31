#include "rand.hpp"

rand::SysRand::SysRand(int seed): Seed(seed) {
    engine = std::default_random_engine(seed);
    Dist = std::uniform_int_distribution<int>();
}

int rand::SysRand::Int() {
    return Dist(engine);
}

void rand::SysRand::NewSeed(int seed) {
    engine = std::default_random_engine(seed);
}
