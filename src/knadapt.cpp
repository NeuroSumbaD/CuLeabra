#include "knadapt.hpp"

knadapt::Chan::Chan(bool on, float rise, float max, float tau): On(on), Rise(rise), Max(max), Tau(tau) {
    Update();
}

void knadapt::Chan::Defaults()
{
    On = true;
    Rise = 0.01;
    Max = 0.1;
    Tau = 100;
    Update();
}

void knadapt::Chan::GcFmSpike(float *gKNa, bool spike) {
    if (On) {
		if (spike) {
			*gKNa += Rise * (Max - *gKNa);
		} else {
			*gKNa -= Dt * *gKNa;
		}
	} else {
		*gKNa = 0;
	}
}

void knadapt::Chan::GcFmRate(float *gKNa, float act) {
    if (On) {
		*gKNa += act*Rise*(Max-*gKNa) - (Dt * *gKNa);
	} else {
		*gKNa = 0;
	}
}

knadapt::Params::Params(bool on, float rate) {
    On = on;
    Rate = rate;

    Fast = Chan(true, 0.05, 0.1, 50);

    Med = Chan(true, 0.02, 0.1, 200);
    
    Slow = Chan(true, 0.001, 1, 1000);

    // Update(); Redundant because Chan calls its own update
}

void knadapt::Params::Defaults() {
    On = true;
    Rate = 0.8;

    Fast = Chan();
    Fast.Tau = 50;
    Fast.Rise = 0.05;
    Fast.Max = 0.1;

    Med = Chan();
    Med.Tau = 200;
    Med.Rise = 0.02;
    Med.Max = 0.1;
    Slow = Chan();
    Slow.Tau = 1000;
    Slow.Rise = 0.001;
    Slow.Max = 1;

    Update();
}

void knadapt::Params::Update() {
    Fast.Update();
    Med.Update();
    Slow.Update();
}

void knadapt::Params::GcFromSpike(float *gKNaF, float *gKNaM, float *gKNaS, bool spike) {
    Fast.GcFmSpike(gKNaF, spike);
	Med.GcFmSpike(gKNaM, spike);
	Slow.GcFmSpike(gKNaS, spike);
}

void knadapt::Params::GcFromRate(float *gKNaF, float *gKNaM, float *gKNaS, float act) {
    Fast.GcFmSpike(gKNaF, act);
	Med.GcFmSpike(gKNaM, act);
	Slow.GcFmSpike(gKNaS, act);
}