#include "rand.hpp"
#include <ctime>

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
	}
	return Mean;
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
float rands::BinomialGen(float n, float p, SysRand &rnd) {
    auto dist = std::binomial_distribution(n, p);
    return dist(rnd.engine);
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
