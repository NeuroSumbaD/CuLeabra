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


    // SysRand is a wrapper for the system random number generator
    struct SysRand {
        int Seed;
        std::default_random_engine engine;
        // std::uniform_int_distribution<int> Dist;

        SysRand();
        SysRand(int seed);
        int Int();
        float Float();
        void NewSeed(int seed);
    };

    SysRand *NewGlobalRand(){return &SysRand();};

    // Provides parameterized random number generation according to different distributions
    // and variance, mean params
    struct Dist {
        float Mean;
        float Var;
        float Par;
        RandDists DistType;

        // TODO: Check if this default initialization works
        Dist(float mean=0, float var=1, float par=1, RandDists type=RandDists::Mean): Mean(mean), Var(var), Par(par), DistType(type){};

        float Gen(SysRand &rnd = *NewGlobalRand());
    };
    

    // RANDOM NUMBER GENERATORS

    float UniformMeanRange(float mean, float rnge, SysRand &rnd = *NewGlobalRand());
    float BinomialGen(float n, float p, SysRand &rnd = *NewGlobalRand());
    float PoissonGen(float lambda, SysRand &rnd = *NewGlobalRand());
    float GammaGen(float alpha, float beta, SysRand &rnd = *NewGlobalRand());
    float GaussianGen(float mean, float sigma, SysRand &rnd = *NewGlobalRand());
    float BetaGen(float alpha, float beta, SysRand &rnd = *NewGlobalRand());
} // namespace rand
