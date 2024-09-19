/*
Package knadapt provides code for sodium (Na) gated potassium (K) currents that drive
adaptation (accommodation) in neural firing.  As neurons spike, driving an influx of Na,
this activates the K channels, which, like leak channels, pull the membrane potential
back down toward rest (or even below).  Multiple different time constants have been
identified and this implementation supports 3:
M-type (fast), Slick (medium), and Slack (slow)

Here's a good reference:

Kaczmarek, L. K. (2013). Slack, Slick, and Sodium-Activated Potassium Channels.
ISRN Neuroscience, 2013. https://doi.org/10.1155/2013/354262

This package supports both spiking and rate-coded activations.
*/
#pragma once
#include "params.hpp"

namespace knadapt {
    struct Chan: params::StylerObject {
        bool On; // if On, use this component of K-Na adaptation
        float Rise; // Rise rate of fast time-scale adaptation as function of Na concentration -- directly multiplies -- 1/rise = tau for rise rate
        float Max; // Maximum potential conductance of fast K channels -- divide nA biological value by 10 for the normalized units here
        float Tau; // time constant in cycles for decay of adaptation, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life)
        float Dt; // 1/Tau rate constant
        Chan(bool on=true, float rise = 0.01, float max=0.1, float tau=100);

        void Update(){Dt = 1/Tau;};
        void Defaults();
        void GcFmSpike(float* gKNa, bool spike);
        void GcFmRate(float* gKNa, float act);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };
    
    // Params describes sodium-gated potassium channel adaptation mechanism.
    // Evidence supports at least 3 different time constants:
    // M-type (fast), Slick (medium), and Slack (slow)
    struct Params: params::StylerObject {
        bool On; // if On, apply K-Na adaptation
        float Rate; // extra multiplier for rate-coded activations on rise factors -- adjust to match discrete spiking
        Chan Fast; // fast time-scale adaptation
        Chan Med; // medium time-scale adaptation
        Chan Slow; // slow time-scale adaptation

        Params(){Defaults();};
        Params(bool on = true, float rate = 0.8);
        
        void Defaults();
        void Update();
        void GcFromSpike(float* gKNaF, float* gKNaM, float* gKNaS, bool spike);
        void GcFromRate(float* gKNaF, float* gKNaM, float* gKNaS, float act);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };
    
}