#pragma once
#include "fffb.hpp"
#include "act.hpp"

namespace inhib{
    // SelfInhibParams defines parameters for Neuron self-inhibition -- activation of the neuron directly feeds back
    // to produce a proportional additional contribution to Gi
    struct SelfInhibParams{
        bool On; // enable neuron self-inhibition
        float Gi; // strength of individual neuron self feedback inhibition -- can produce proportional activation behavior in individual units for specialized cases (e.g., scalar val or BG units), but not so good for typical hidden layers
        float Tau; // time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life) for integrating unit self feedback inhibitory values -- prevents oscillations that otherwise occur -- relatively rapid 1.4 typically works, but may need to go longer if oscillations are a problem
        float Dt; // rate = 1/ tau
        SelfInhibParams(bool On = false, float Gi = 0.4, float Tau = 1.4);
        void Update();
        void Inhib(float* self, float act);
    };
    
    // ActAvgParams represents expected average activity levels in the layer.
    // Used for computing running-average computation that is then used for netinput scaling.
    // Also specifies time constant for updating average
    // and for the target value for adapting inhibition in inhib_adapt.
    struct ActAvgParams {
        float Init; // [min: 0] [typically 0.1 - 0.2] initial estimated average activity level in the layer (see also UseFirst option -- if that is off then it is used as a starting point for running average actual activity level, ActMAvg and ActPAvg) -- ActPAvg is used primarily for automatic netinput scaling, to balance out layers that have different activity levels -- thus it is important that init be relatively accurate -- good idea to update from recorded ActPAvg levels
        bool Fixed; // [def: false] if true, then the Init value is used as a constant for ActPAvgEff (the effective value used for netinput rescaling), instead of using the actual running average activation
        bool UseExtAct; // [def: false] if true, then use the activation level computed from the external inputs to this layer (avg of targ or ext unit vars) -- this will only be applied to layers with Input or Target / Compare layer types, and falls back on the targ_init value if external inputs are not available or have a zero average -- implies fixed behavior
        bool UseFirst; // [def: true] use the first actual average value to override targ_init value -- actual value is likely to be a better estimate than our guess
        float Tau; // [def: 100] [min: 1] time constant in trials for integrating time-average values at the layer level -- used for computing Pool.ActAvg.ActsMAvg, ActsPAvg
        float Adjust; // [def: 1] adjustment multiplier on the computed ActPAvg value that is used to compute ActPAvgEff, which is actually used for netinput rescaling -- if based on connectivity patterns or other factors the actual running-average value is resulting in netinputs that are too high or low, then this can be used to adjust the effective average activity value -- reducing the average activity with a factor < 1 will increase netinput scaling (stronger net inputs from layers that receive from this layer), and vice-versa for increasing (decreases net inputs)
        float Dt; // rate = 1 / tau
        ActAvgParams(float Init = 0.15, bool Fixed = false, bool UseExtAct = false, bool Usefirst = true, float Tau = 100, float Adjust = 1);
        void Update();
        float EffInit();
        void AvgFmAct(float* avg, float act);
        void EffFmAvg(float* eff, float avg);
    };
    
    // InhibParams contains all the inhibition computation params and functions for basic Leabra
    // This is included in leabra.Layer to support computation.
    // This also includes other misc layer-level params such as running-average activation in the layer
    // which is used for netinput rescaling and potentially for adapting inhibition over time
    struct InhibParams {
        fffb::Params Layer; // inhibition across the entire layer
        fffb::Params Pool;
        SelfInhibParams Self;
        ActAvgParams ActAvg;
        InhibParams();
    };
};