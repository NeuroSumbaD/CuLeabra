#include "nxx1.hpp"
#include <cmath>


// C++ Constructor for params
nxx1::Params::Params(float Thr, float Gain, float NVar, float VmActThr,
                     float SigMult, float SigMultPow, float SigGain,
                     float InterpRange, float GainCorRange, float GainCor):
    Thr(Thr), Gain(Gain), NVar(NVar), VmActThr(VmActThr), SigMult(SigMult),
    SigMultPow(SigMultPow), SigGain(SigGain), InterpRange(InterpRange),
    GainCorRange(GainCorRange), GainCor(GainCor){ // TODO: Initializer list is ugly... find cleaner way to do this
    Update(); // Initializes derived member variables
}

void nxx1::Params::Defaults(){
    Thr = 0.5;
    Gain = 100.0;
    NVar = 0.005;
    VmActThr = 0.01;
    SigMult = 0.33;
    SigMultPow = 0.8;
    SigGain = 3.0;
    InterpRange = 0.01;
    GainCorRange = 10.0;
    GainCor = 0.1;
    Update();
}


// Update function for initializing derived NXX1 params
void nxx1::Params::Update() {
    this->SigGainNVar = SigGain / NVar;
    this->SigMultEff = SigMult * std::pow(Gain * NVar, SigMultPow);
    this->SigValAt0 = 0.5 * SigMultEff;
    this->InterpVal = XX1GainCor(InterpRange) - SigValAt0;
}

// XX1 computes the basic x/(x+1) function
float nxx1::Params::XX1(float x) {
    return x / (x+1);
}

// XX1GainCor computes x/(x+1) with gain correction within GainCorRange to compensate for convolution effects
float nxx1::Params::XX1GainCor(float x) {
    float gainCorFact = (GainCorRange - (x / this->NVar)) / this->GainCorRange;
    if (gainCorFact < 0) {
        return XX1(Gain * x);
    }
    else {
        float newGain = Gain * (1 - GainCor * gainCorFact);
        return XX1(newGain * x);
    }
}


// NoisyXX1 computes the Noisy x/(x+1) function -- directly computes close approximation
// to x/(x+1) convolved with a gaussian noise function with variance nvar.
// No need for a lookup table -- very reasonable approximation for standard range of parameters
// (nvar = .01 or less -- higher values of nvar are less accurate with large gains,
// but ok for lower gains)
float nxx1::Params::NoisyXX1(float x)
{
    if (x < 0){
        float ex = - (x * SigGainNVar);
        if (ex > 50) {
            return 0;
        }
        return SigMultEff / (1 + std::exp(ex));
    }
    else if (x < InterpRange) {
        float interp = 1 - ((InterpRange - x)/ InterpRange);
        return SigValAt0 + interp * InterpVal;
    }
    else {
        return XX1GainCor(x);
    }
}

// XX1GainCorGain computes x/(x+1) with gain correction within GainCorRange
// to compensate for convolution effects -- using external gain factor
float nxx1::Params::XX1GainCorGain(float x, float gain) {
    float gainCorFact = (GainCorRange - (x / NVar)) / GainCorRange;
    if (gainCorFact < 0) {
        return XX1(gain * x);
    }
    float newGain = gain * (1 - GainCor*gainCorFact);
    return XX1(newGain * x);
}

// NoisyXX1Gain computes the noisy x/(x+1) function -- directly computes close approximation
// to x/(x+1) convolved with a gaussian noise function with variance nvar.
// No need for a lookup table -- very reasonable approximation for standard range of parameters
// (nvar = .01 or less -- higher values of nvar are less accurate with large gains,
// but ok for lower gains).  Using external gain factor.
float nxx1::Params::NoisyXX1Gain(float x, float gain) {
    if (x < InterpRange) {
        float sigMultEffArg = SigMult * std::pow(gain * NVar, SigMultPow);
        float sigValAt0Arg = 0.5 * sigMultEffArg;

        if (x < 0) {
            float ex = -(x * SigGainNVar);
            if (ex > 50) {
                return 0;
            }
            return sigMultEffArg / (1 + std::exp(ex));
        }
        else {
            float interp = 1 - ((InterpRange - x) / InterpRange);
            return sigValAt0Arg + interp*InterpVal;
        }
    }
    else {
        return XX1GainCorGain(x, gain);
    }
}
