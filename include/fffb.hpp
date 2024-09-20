#pragma once
#include "minmax.hpp"
#include "params.hpp"

namespace fffb {
    // Inhib contains state values for computed FFFB inhibition
    struct Inhib: params::StylerObject {
        float FFi;
        float FBi;
        float Gi;
        float GiOrig;
        float LayGi;
        minmax::AvgMax32 Ge;
        minmax::AvgMax32 Act;
        Inhib();
        void Zero();
        void Decay(float decay);

        void Init();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~Inhib() = default;
    };
    
    // Params parameterizes feedforward (FF) and feedback (FB) inhibition (FFFB)
    // based on average (or maximum) netinput (FF) and activation (FB)
    struct Params: params::StylerObject {
        bool On; // enable this level of inhibition
        float Gi; // [def: 1.8] [min: 0] [1.5-2.3 typical, can go lower or higher as needed] overall inhibition gain -- this is main parameter to adjust to change overall activation levels -- it scales both the the ff and fb factors uniformly
        float FF; // [def: 1] [min: 0] overall inhibitory contribution from feedforward inhibition -- multiplies average netinput (i.e., synaptic drive into layer) -- this anticipates upcoming changes in excitation, but if set too high, it can make activity slow to emerge -- see also ff0 for a zero-point for this value
        float FB; // [def: 1] [min: 0] overall inhibitory contribution from feedback inhibition -- multiplies average activation -- this reacts to layer activation levels and works more like a thermostat (turning up when the 'heat' in the layer is too high)
        float FBTau; // [def: 1.4,3,5] [min: 0] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life) for integrating feedback inhibitory values -- prevents oscillations that otherwise occur -- the fast default of 1.4 should be used for most cases but sometimes a slower value (3 or higher) can be more robust, especially when inhibition is strong or inputs are more rapidly changing
        float MaxVsAvg; // [def: 0,0.5,1] what proportion of the maximum vs. average netinput to use in the feedforward inhibition computation -- 0 = all average, 1 = all max, and values in between = proportional mix between average and max (ff_netin = avg + ff_max_vs_avg * (max - avg)) -- including more max can be beneficial especially in situations where the average can vary significantly but the activity should not -- max is more robust in many situations but less flexible and sensitive to the overall distribution -- max is better for cases more closely approximating single or strictly fixed winner-take-all behavior -- 0.5 is a good compromise in many cases and generally requires a reduction of .1 or slightly more (up to .3-.5) from the gi value for 0
        float FF0; // [def: 0.1] feedforward zero point for average netinput -- below this level, no FF inhibition is computed based on avg netinput, and this value is subtraced from the ff inhib contribution above this value -- the 0.1 default should be good for most cases (and helps FF_FB produce k-winner-take-all dynamics), but if average netinputs are lower than typical, you may need to lower it
        float FBDt; //  rate = 1 / tau
        // Params(){Defaults();}; // redundant
        Params(float Gi = 1.8, float FF = 1, float FB = 1, float FBTau = 1.4, float MaxVsAvg = 0, float FF0 = 0.1);

        void Update();
        void Defaults();
        float FFInhib(float avgGe, float maxGe);
        float FBInhib(float avgAct);
        void FBUpdt(float* fbi, float newFbi);
        void Inhib(Inhib* inh);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~Params() = default;
    };
    
};