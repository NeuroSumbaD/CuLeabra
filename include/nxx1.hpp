#pragma once
#include "params.hpp"

namespace nxx1{
    struct Params: params::StylerObject {
        float Thr; // threshold value Theta (Q) for firing output activation (.5 is more accurate value based on AdEx biological parameters and normalization
        float Gain; // gain (gamma) of the rate-coded activation functions -- 100 is default, 80 works better for larger models, and 20 is closer to the actual spiking behavior of the AdEx model -- use lower values for more graded signals, generally in lower input/sensory layers of the network
        float NVar; // variance of the Gaussian noise kernel for convolving with XX1 in NOISY_XX1 and NOISY_LINEAR -- determines the level of curvature of the activation function near the threshold -- increase for more graded responding there -- note that this is not actual stochastic noise, just constant convolved gaussian smoothness to the activation function
        float VmActThr; // threshold on activation below which the direct vm - act.thr is used -- this should be low -- once it gets active should use net - g_e_thr ge-linear dynamics (gelin)
        float SigMult; // multiplier on sigmoid used for computing values for net < thr
        float SigMultPow; // power for computing sig_mult_eff as function of gain * nvar
        float SigGain; // gain multipler on (net - thr) for sigmoid used for computing values for net < thr
        float InterpRange; // interpolation range above zero to use interpolation
        float GainCorRange; // range in units of nvar over which to apply gain correction to compensate for convolution
        float GainCor; // gain correction multiplier -- how much to correct gains
        float SigGainNVar; // sig_gain / nvar
        float SigMultEff; // overall multiplier on sigmoidal component for values below threshold = sig_mult * pow(gain * nvar, sig_mult_pow)
        float SigValAt0; // 0.5 * sig_mult_eff -- used for interpolation portion
        float InterpVal; // function value at interp_range - sig_val_at_0 -- for interpolation
        Params(
            float Thr = 0.5,
            float Gain = 100.0,
            float NVar = 0.005,
            float VmActThr = 0.01,
            float SigMult = 0.33,
            float SigMultPow = 0.8,
            float SigGain = 3.0,
            float InterpRange = 0.01,
            float GainCorRange = 10.0,
            float GainCor = 0.1
        );
        void Update();
        void Defaults();
        float XX1GainCor(float x);
        float XX1(float x);
        float NoisyXX1(float x);
        float XX1GainCorGain(float x, float gain);
        float NoisyXX1Gain(float x, float gain);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };



}