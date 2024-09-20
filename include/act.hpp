/*
act.go contains the activation params and functions for leabra

leabra.ActParams contains all the activation computation params and functions
for basic Leabra, at the neuron level .
This is included in leabra.Layer to drive the computation. 
*/

#pragma once

#include "nxx1.hpp"
#include "chans.hpp"
#include "minmax.hpp"
#include "knadapt.hpp"
#include "neuron.hpp"
#include "rand.hpp"
#include "params.hpp"

namespace leabra{

    // OptThreshParams provides optimization thresholds for faster processing
    struct OptThreshParams: params::StylerObject {
        float Send; // don't send activation when act <= send -- greatly speeds processing
        float Delta; // don't send activation changes until they exceed this threshold: only for when LeabraNetwork::send_delta is on!
        OptThreshParams(float Send = 0.1, float Delta = 0.005);

        void Defaults();
        void Update();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~OptThreshParams() = default;
    };

    // ActInitParams are initial values for key network state variables.
    // Initialized at start of trial with Init_Acts or DecayState.
    struct ActInitParams: params::StylerObject {
        float Decay; // proportion to decay activation state toward initial values at start of every trial
        float Vm; // initial membrane potential -- see e_rev.l for the resting potential (typically .3) -- often works better to have a somewhat elevated initial membrane potential relative to that
        float Act; // initial activation value -- typically 0
        float Ge; // baseline level of excitatory conductance (net input) -- Ge is initialized to this value, and it is added in as a constant background level of excitatory input -- captures all the other inputs not represented in the model, and intrinsic excitability, etc
        ActInitParams(float Decay = 1, float Vm = 0.4, float Act = 0, float Ge = 0);

        void Defaults();
        void Update();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~ActInitParams() = default;
    };

    // DtParams are time and rate constants for temporal derivatives in Leabra (Vm, net input)
    struct DtParams: params::StylerObject {
        float Integ;
        float VmTau;
        float GTau;
        float AvgTau;
        float VmDt;
        float GDt;
        float AvgDt;

        DtParams(float Integ = 1, float VmTau = 3.3, float AvgTau = 200);
        void Update();
        void GFromRaw(float geRaw, float &ge);

        void Defaults();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~DtParams() = default;
    };

    // ClampParams are for specifying how external inputs are clamped onto network activation values
    struct ClampParams: params::StylerObject {
        bool Hard; // whether to hard clamp inputs where activation is directly set to external input value (Act = Ext) or do soft clamping where Ext is added into Ge excitatory current (Ge += Gain * Ext)
        minmax::F32 Range; // range of external input activation values allowed -- Max is .95 by default due to saturating nature of rate code activation function
        float Gain; // soft clamp gain factor (Ge += Gain * Ext)
        bool Avg; // compute soft clamp as the average of current and target netins, not the sum -- prevents some of the main effect problems associated with adding external inputs
        float AvgGain; // gain factor for averaging the Ge -- clamp value Ext contributes with AvgGain and current Ge as (1-AvgGain)

        ClampParams(bool Hard = true, float RangeMax = 0.95, float Gain = 0.2, bool Avg = false, float AvgGain = 0.2);
        float AvgGe(float ext, float ge);

        void Defaults();
        void Update();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~ClampParams() = default;
    };

    struct WtInitParams: rands::Dist {
        // float Mean;
        // float Var;
        // std::uniform_real_distribution<float> Type;
        bool Sym;

        WtInitParams(float mean = 0.5, float var = 0.25, float par=1, rands::RandDists type=rands::Uniform);

        void Defaults();
    };

    struct WtScaleParams: params::StylerObject{
        float Abs; // absolute scaling, which is not subject to normalization: directly multiplies weight values
        float Rel; // relative scaling that shifts balance between different pathways -- this is subject to normalization across all other pathways into unit

        WtScaleParams(float abs = 1, float rel = 1);

        void Defaults();
        void Update();

        float SLayActScale(float savg, float snu, float ncon);
        float FullScale(float savg, float snu, float ncon);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~WtScaleParams() = default;
    };

    enum ActNoiseType {
        NoNoise, // NoNoise means no noise added
        VmNoise, // VmNoise means noise is added to the membrane potential. IMPORTANT: this should NOT be used for rate-code (NXX1) activations, because they do not depend directly on the vm -- this then has no effect
        GeNoise, // GeNoise means noise is added to the excitatory conductance (Ge). This should be used for rate coded activations (NXX1)
        ActNoise, // ActNoise means noise is added to the final rate code activation
        GeMultNoise, // GeMultNoise means that noise is multiplicative on the Ge excitatory conductance values
        ActNoiseTypeN,
    };

    // ActNoiseParams contains parameters for activation-level noise
    struct ActNoiseParams: rands::Dist, params::StylerObject {

        ActNoiseType Type; // where and how to add processing noise (should be an enum)
        bool Fixed; // keep the same noise value over the entire alpha cycle -- prevents noise from being washed out and produces a stable effect that can be better used for learning -- this is strongly recommended for most learning situations
        
        ActNoiseParams();
        ActNoiseParams(ActNoiseType Type, bool Fixed=true);

        void Defaults();
        void Update();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };


    
    // leabra.ActParams contains all the activation computation params and functions
    // for basic Leabra, at the neuron level .
    // This is included in leabra.Layer to drive the computation.
    struct ActParams: params::StylerObject {
        nxx1::Params XX1; //Noisy X/X+1 rate code activation function parameters
        OptThreshParams OptThresh; // optimization thresholds for faster processing
        ActInitParams Init; // initial values for key network state variables -- initialized at start of trial with InitActs or DecayActs
        DtParams Dt; // time and rate constants for temporal derivatives / updating of activation state
        chans::Chans Gbar; // maximal conductances levels for channels
        chans::Chans Erev; // reversal potentials for each channel
        ClampParams Clamp; // // how external inputs drive neural activations
        ActNoiseParams Noise; // how, where, when, and how much noise to add to activations
        minmax::F32 VmRange; // range for Vm membrane potential -- [0, 2.0] by default
        knadapt::Params KNa; // sodium-gated potassium channel adaptation parameters -- activates an inhibitory leak-like current as a function of neural activity (firing = Na influx) at three different time-scales (M-type = fast, Slick = medium, Slack = slow)
        chans::Chans ErevSubThr; // Erev - Act.Thr for each channel -- used in computing GeThrFmG among others
        chans::Chans ThrSubErev; // Act.Thr - Erev for each channel -- used in computing GeThrFmG among others

        ActParams();
        void Defaults();
        void Update();

        void InitGInc(Neuron &nrn);
        void DecayState(Neuron &nrn, float decay);
        void InitActs(Neuron &nrn);
        void InitActQs(Neuron &nrn);

        //Cycle
        void GeFromRaw(Neuron &nrn, float geRaw);
        void GiFromRaw(Neuron &nrn, float giRaw);
        float InetFromG(float vm, float ge, float gi, float gk);
        void VmFromG(Neuron &nrn);
        float GeThrFromG(Neuron &nrn);
        float GeThrFromGnoK(Neuron &nrn);
        void ActFromG(Neuron &nrn);
        bool HasHardClamp(Neuron &nrn);
        void HardClamp(Neuron &nrn);
        
        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~ActParams() = default;
    };
    
}