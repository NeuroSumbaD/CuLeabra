#include "rand.hpp"
#include <ctime>
#include <iostream>
#include <algorithm>
#include <numeric> 

namespace rands {
    SysRand* globalRandGenerator = nullptr;
} // namespace rands


rands::SysRand::SysRand() {
    int seed = time(0);
    engine = std::default_random_engine(seed);
}

rands::SysRand::SysRand(int seed) : Seed(seed) {
    engine = std::default_random_engine(seed);
    // Dist = std::uniform_int_distribution<int>();
}

//
int rands::SysRand::Int() {
    auto dist = std::uniform_int_distribution<int>();
    return dist(engine);
}

float rands::SysRand::Float() {
    auto dist = std::uniform_real_distribution();
    return dist(engine);
}

void rands::SysRand::NewSeed(int seed) {
    engine = std::default_random_engine(seed);
}

// Intn returns, as an int, a non-negative pseudo-random number in the half-open interval [0,n).
// It panics if n <= 0.
uint rands::SysRand::Intn(uint n) {
    if (n <= 0) {
		throw std::invalid_argument("Invalid argument to Intn");
	}
	if (n > (((uint)1<<31)-1)) {
        throw std::invalid_argument("Argument to Intn greater than 32 bits");
	}
    auto dist = std::uniform_int_distribution<uint>(0,n);
    return dist(engine);
	// return int(Int63n(int(n)));
}

// Perm returns, as a slice of n ints, a pseudo-random permutation of the integers
// in the half-open interval [0,n).
std::vector<int> rands::SysRand::Perm(int n) {
    // std::vector<int> m(n);
    // In the following loop, the iteration when i=0 always swaps m[0] with m[0].
	// A change to remove this useless iteration is to assign 1 to i in the init
	// statement. But Perm also effects r. Making this change will affect
	// the final state of r. So this change can't be made for compatibility
	// reasons for Go 1.
	// for (int i = 0; i < n; i++) {
	// 	int j = Intn(i + 1);
	// 	m[i] = m[j];
	// 	m[j] = i;
	// }
    // Above matches leabra

    // Below uses c++ standard library
    std::vector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);
    // Shuffle the vector to get a random permutation
    std::shuffle(perm.begin(), perm.end(), engine);
    return perm;
}

rands::Dist::Dist(float mean, float var, float par, RandDists type): Mean(mean), Var(var), Par(par), DistType(type){}

// Gen generates a random variable according to current parameters.
// Optionally can pass a single SysRand interface to use --
// otherwise uses system global SysRand source.
float rands::Dist::Gen(SysRand &rnd) {
    // var rnd Rand
	// if len(randOpt) == 0 {
	// 	rnd = NewGlobalRand()
	// } else {
	// 	rnd = randOpt[0]
	// }
	switch (DistType) {
        case Uniform:
            return UniformMeanRange(Mean, Var, rnd);
            break;
        case Binomial:
            return Mean + BinomialGen(Par, Var, rnd);
            break;
        case Poisson:
            return Mean + PoissonGen(Var, rnd);
        case Gamma:
            return Mean + GammaGen(Par, Var, rnd);
        case Gaussian:
            return GaussianGen(Mean, Var, rnd);
        case Beta:
            return Mean + BetaGen(Var, Par, rnd);
        case RandDists::Mean:
            return Mean;
	}
	return Mean;
}


rands::SysRand *rands::NewGlobalRand(){
    if (globalRandGenerator == nullptr) {
        globalRandGenerator = new SysRand();
    }
    return globalRandGenerator;
}

// UniformMeanRange returns uniform random number with given range on either size of the mean:
// [mean - range, mean + range]
float rands::UniformMeanRange(float mean, float rnge, SysRand &rnd) {
    auto dist = std::uniform_real_distribution(mean-rnge, mean+rnge);
    return dist(rnd.engine);
}

// BinomialGen returns binomial with n trials (par) each of probability p (var)
// Optionally can pass a single Rand interface to use --
// otherwise uses system global Rand source.
// TODO: Double-check that this is the expected result of BinomialGen
float rands::BinomialGen(float n, float p, SysRand &rnd) {
    auto dist = std::binomial_distribution<int>(n, p);
    return (float)dist(rnd.engine);
    // return ((float)dist(rnd.engine))/n;
}

// PoissonGen returns poisson variable, as number of events in interval,
// with event rate (lmb = Var) plus mean
float rands::PoissonGen(float lambda, SysRand &rnd) {
    auto dist = std::poisson_distribution(lambda);
    return dist(rnd.engine);
}

// GammaGen represents maximum entropy distribution with two parameters:
// a shape parameter (Alpha, Par in RandParams),
// and a scaling parameter (Beta, Var in RandParams).
float rands::GammaGen(float alpha, float beta, SysRand &rnd) {
    auto dist = std::gamma_distribution(alpha, beta);
    return dist(rnd.engine);
}

// GaussianGen returns gaussian (normal) random number with given
// mean and sigma standard deviation.
float rands::GaussianGen(float mean, float sigma, SysRand &rnd) {
    auto dist = std::normal_distribution<float>(mean, sigma);
    return dist(rnd.engine);
}

// BetaGen returns beta random number with two shape parameters
// alpha > 0 and beta > 0
float rands::BetaGen(float alpha, float beta, SysRand &rnd) {
    float ga = GammaGen(alpha, 1, rnd);
    float gb = GammaGen(beta, 1, rnd);
    return ga / (ga + gb);
}

std::vector<int> rands::Perm(int n) {
    SysRand *rnd = NewGlobalRand();
    return rnd->Perm(n);
}
