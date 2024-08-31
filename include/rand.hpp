#pragma once
#include <cstdlib>
#include <random>
#include <any>

namespace rands {
    // RandDists are different random number distributions
    enum RandDists{
        // Uniform has a uniform probability distribution over Var = range on either side of the Mean
        Uniform,

        // Binomial represents number of 1's in n (Par) random (Bernouli) trials of probability p (Var)
        Binomial,

        // Poisson represents number of events in interval, with event rate (lambda = Var) plus Mean
        Poisson,

        // Gamma represents maximum entropy distribution with two parameters: scaling parameter (Var)
        // and shape parameter k (Par) plus Mean
        Gamma,

        // Gaussian normal with Var = stddev plus Mean
        Gaussian,

        // Beta with Var = alpha and Par = beta shape parameters
        Beta,

        // Mean is just the constant Mean, no randomness
        Mean
    };

    struct Dist {
        float Mean;
        float Var;
        float Par;
        RandDists Type;

        Dist(float mean, float var, float par, RandDists type): Mean(mean), Var(var), Par(par), Type(type){};
    };

    // SysRand is a wrapper for the system random number generator
    struct SysRand {
        int Seed;
        std::default_random_engine engine;
        std::uniform_int_distribution<int> Dist;

        SysRand(int seed = 0);
        int Int();
        void NewSeed(int seed);
    };

    
} // namespace rand
