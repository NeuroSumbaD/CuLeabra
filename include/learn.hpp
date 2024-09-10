#pragma once
#include <tuple>
#include "neuron.hpp"
#include "synapse.hpp"

namespace leabra {

    // LrnActAvgParams has rate constants for averaging over activations at different time scales,
    // to produce the running average activation values that then drive learning in the XCAL learning rules
    struct LrnActAvgParams{
        float SSTau; // [def: 2,4,7] [min: 1] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life), for continuously updating the super-short time-scale avg_ss value -- this is provides a pre-integration step before integrating into the avg_s short time scale -- it is particularly important for spiking -- in general 4 is the largest value without starting to impair learning, but a value of 7 can be combined with m_in_s = 0 with somewhat worse results
        float STau; // [def: 2] [min: 1] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life), for continuously updating the short time-scale avg_s value from the super-short avg_ss value (cascade mode) -- avg_s represents the plus phase learning signal that reflects the most recent past information
        float MTau; // [def: 10] [min: 1] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life), for continuously updating the medium time-scale avg_m value from the short avg_s value (cascade mode) -- avg_m represents the minus phase learning signal that reflects the expectation representation prior to experiencing the outcome (in addition to the outcome) -- the default value of 10 generally cannot be exceeded without impairing learning
        float LrnM; // [def: 0.1,0] [min: 0] [max: 1] how much of the medium term average activation to mix in with the short (plus phase) to compute the Neuron AvgSLrn variable that is used for the unit's short-term average in learning. This is important to ensure that when unit turns off in plus phase (short time scale), enough medium-phase trace remains so that learning signal doesn't just go all the way to 0, at which point no learning would take place -- typically need faster time constant for updating S such that this trace of the M signal is lost -- can set SSTau=7 and set this to 0 but learning is generally somewhat worse
        float Init; // [def: 0.15] [min: 0] [max: 1] initial value for average
        float SSDt; // [view: -] rate = 1 / tau
        float SDt; // [view: -] rate = 1 / tau
        float MDt; // [view: -] rate = 1 / tau
        float LrnS; // [view: -] 1-LrnM

        LrnActAvgParams(float SSTau=2.0, float STau=2.0, float MTau=10.0, float LrnM=0.1, float Init=0.15):
            SSTau(SSTau),STau(STau),MTau(MTau),LrnM(LrnM),Init(Init)
            {Update();};

        void AvgsFromAct(float ruAct, float &avgSS, float &avgS, float &avgM, float &avgSLrn);

        void Update(){SSDt = 1/SSTau; SDt=1/STau; MDt = 1/MTau; LrnS = 1 - LrnM;};
        void Defaults();
    };

    // AvgLParams are parameters for computing the long-term floating average value, AvgL
    // which is used for driving BCM-style hebbian learning in XCAL -- this form of learning
    // increases contrast of weights and generally decreases overall activity of neuron,
    // to prevent "hog" units -- it is computed as a running average of the (gain multiplied)
    // medium-time-scale average activation at the end of the alpha-cycle.
    // Also computes an adaptive amount of BCM learning, AvgLLrn, based on AvgL.
    struct AvgLParams {
        float Init; // [def: 0.4] [min: 0] [max: 1] initial AvgL value at start of training
        float Gain; // [def: 1.5,2,2.5,3,4,5] [min: 0] gain multiplier on activation used in computing the running average AvgL value that is the key floating threshold in the BCM Hebbian learning rule -- when using the DELTA_FF_FB learning rule, it should generally be 2x what it was before with the old XCAL_CHL rule, i.e., default of 5 instead of 2.5 -- it is a good idea to experiment with this parameter a bit -- the default is on the high-side, so typically reducing a bit from initial default is a good direction
        float Min; // [def: 0.2] [min: 0] miniumum AvgL value -- running average cannot go lower than this value even when it otherwise would due to inactivity -- default value is generally good and typically does not need to be changed
        float Tau; // [def: 10] [min: 1] time constant for updating the running average AvgL -- AvgL moves toward gain*act with this time constant on every alpha-cycle - longer time constants can also work fine, but the default of 10 allows for quicker reaction to beneficial weight changes
        float LrnMax; // [def: 0.5] [min: 0] maximum AvgLLrn value, which is amount of learning driven by AvgL factor -- when AvgL is at its maximum value (i.e., gain, as act does not exceed 1), then AvgLLrn will be at this maximum value -- by default, strong amounts of this homeostatic Hebbian form of learning can be used when the receiving unit is highly active -- this will then tend to bring down the average activity of units -- the default of 0.5, in combination with the err_mod flag, works well for most models -- use around 0.0004 for a single fixed value (with err_mod flag off)
        float LrnMin; // [def: 0.0001,0.0004] [min: 0] miniumum AvgLLrn value (amount of learning driven by AvgL factor) -- if AvgL is at its minimum value, then AvgLLrn will be at this minimum value -- neurons that are not overly active may not need to increase the contrast of their weights as much -- use around 0.0004 for a single fixed value (with err_mod flag off)
        bool ErrMod; // [def: true] modulate amount learning by normalized level of error within layer
        float ModMin; // [def: 0.01]  minimum modulation value for ErrMod-- ensures a minimum amount of self-organizing learning even for network / layers that have a very small level of error signal
        float Dt; // rate = 1 / tau
        float LrnFact; // (LrnMax - LrnMin) / (Gain - Min)

        AvgLParams(float init=0.4, float gain=2.5, float min=0.2, float tau=10, float lrnMax=0.5, float lrnMin=0.0001, bool errMod=true, float modMin=0.01):
            Init(init),Gain(gain),Tau(tau),LrnMax(lrnMax),LrnMin(lrnMin),ErrMod(errMod),ModMin(modMin)
            {Update();};

        void AvgLFromAvgM(float avgM, float &avgL, float &lrn);
        float ErrModFromLayErr(float layCosDiffAvg);
        void Defaults();
        void Update(){Dt=1/Tau; LrnFact=(LrnMax - LrnMin)/(Gain - Min);};
    };

    // CosDiffParams specify how to integrate cosine of difference between plus and minus phase activations
    // Used to modulate amount of hebbian learning, and overall learning rate.
    struct CosDiffParams{
        float Tau; // [def: 100] [min: 1] time constant in alpha-cycles (roughly how long significant change takes, 1.4 x half-life) for computing running average CosDiff value for the layer, CosDiffAvg = cosine difference between ActM and ActP -- this is an important statistic for how much phase-based difference there is between phases in this layer -- it is used in standard X_COS_DIFF modulation of l_mix in LeabraConSpec, and for modulating learning rate as a function of predictability in the DeepLeabra predictive auto-encoder learning -- running average variance also computed with this: cos_diff_var
        float Dt; // rate constant = 1 / Tau
        float DtC; // complement of rate constant = 1 - Dt

        CosDiffParams(float tau=100):Tau(tau){Update();};

        void AvgVarFromCos(float &avg, float &vr, float cos);

        void Update(){Dt = 1/Tau; DtC = 1 - Dt;};
        void Defaults(){Tau = 100; Update();};
    };

    // CosDiffStats holds cosine-difference statistics at the layer level
    struct CosDiffStats {
        float Cos = 0; // cosine (normalized dot product) activation difference between ActP and ActM on this alpha-cycle for this layer -- computed by CosDiffFmActs at end of QuarterFinal for quarter = 3
        float Avg = 0; // running average of cosine (normalized dot product) difference between ActP and ActM -- computed with CosDiff.Tau time constant in QuarterFinal, and used for modulating BCM Hebbian learning (see AvgLrn) and overall learning rate
        float Var = 0; // running variance of cosine (normalized dot product) difference between ActP and ActM -- computed with CosDiff.Tau time constant in QuarterFinal, used for modulating overall learning rate
        float AvgLrn = 0; // 1 - Avg and 0 for non-Hidden layers
        float ModAvgLLrn = 0; // 1 - AvgLrn and 0 for non-Hidden layers -- this is the value of Avg used for AvgLParams ErrMod modulation of the AvgLLrn factor if enabled

        CosDiffStats(){Init();};
        void Init();
    };
    
    // leabra.LearnNeurParams manages learning-related parameters at the neuron-level.
    // This is mainly the running average activations that drive learning.
    struct LearnNeurParams {
        LrnActAvgParams ActAvg;
        AvgLParams AvgL;
        CosDiffParams CosDiff;

        LearnNeurParams():ActAvg(), AvgL(), CosDiff(){};

        void Update();
        void Defaults();
        void AvgsFromAct(Neuron &nrn);
        void AvgLFromAvgM(Neuron &nrn);

        void InitActAvg(Neuron &nrn);

    };

    // XCalParams are parameters for temporally eXtended Contrastive Attractor Learning function (XCAL)
    // which is the standard learning equation for leabra.
    struct XCalParams{
        // multiplier on learning based on the medium-term floating average threshold which produces error-driven learning -- this is typically 1 when error-driven learning is being used, and 0 when pure Hebbian learning is used. The long-term floating average threshold is provided by the receiving unit
        float MLrn;// `default:"1" min:"0"`

        // if true, set a fixed AvgLLrn weighting factor that determines how much of the long-term floating average threshold (i.e., BCM, Hebbian) component of learning is used -- this is useful for setting a fully Hebbian learning connection, e.g., by setting MLrn = 0 and LLrn = 1. If false, then the receiving unit's AvgLLrn factor is used, which dynamically modulates the amount of the long-term component as a function of how active overall it is
        bool SetLLrn; // `default:"false"`

        // fixed l_lrn weighting factor that determines how much of the long-term floating average threshold (i.e., BCM, Hebbian) component of learning is used -- this is useful for setting a fully Hebbian learning connection, e.g., by setting MLrn = 0 and LLrn = 1.
        float LLrn;

        // proportional point within LTD range where magnitude reverses to go back down to zero at zero -- err-driven svm component does better with smaller values, and BCM-like mvl component does better with larger values -- 0.1 is a compromise
        float DRev; // `default:"0.1" min:"0" max:"0.99"`

        // minimum LTD threshold value below which no weight change occurs -- this is now *relative* to the threshold
        float DThr; // `default:"0.0001,0.01" min:"0"`

        // xcal learning threshold -- don't learn when sending unit activation is below this value in both phases -- due to the nature of the learning function being 0 when the sr coproduct is 0, it should not affect learning in any substantial way -- nonstandard learning algorithms that have different properties should ignore it
        float LrnThr; // `default:"0.01"`

        // -(1-DRev)/DRev -- multiplication factor in learning rule -- builds in the minus sign!
        float DRevRatio;

        XCalParams(float mLrn = 1, bool setLLrn = false, float lLrn = 1, float dRev = 0.1, float dThr = 0.0001, float lrnThr = 0.01):
            MLrn(mLrn), SetLLrn(setLLrn), LLrn(lLrn), DRev(dRev), DThr(dThr), LrnThr(lrnThr){Update();};

        void Update();
        void Defaults();
        float DWt(float srval, float thrP);
        float LongLrate(float avgLLrn);
    };

    // WtSigParams are sigmoidal weight contrast enhancement function parameters
    struct WtSigParams{
        // gain (contrast, sharpness) of the weight contrast function (1 = linear)
        float Gain; // `default:"1,6" min:"0"`

        // offset of the function (1=centered at .5, >1=higher, <1=lower) -- 1 is standard for XCAL
        float Off; // `default:"1" min:"0"`

        // apply exponential soft bounding to the weight changes
        bool SoftBound; // `default:"true"`

        WtSigParams(float gain = 6, float off= 1, bool softBound = true): Gain(gain), Off(off), SoftBound(softBound){Update();};

        void Update(){};
        void Defaults(){Gain = 6, Off = 1; SoftBound = true;};
        float SigFromLinWt(float lw);
        float LinFromSigWt(float sw);
    };
    float SigFun(float w, float gain, float off);
    float SigInvFun(float w, float gain, float off);
    float SigFun61(float w);
    float SigInvFun61(float w);

    struct DWtNormParams {
        // whether to use dwt normalization, only on error-driven dwt component, based on pathway-level max_avg value -- slowly decays and instantly resets to any current max
        bool On; //  `default:"true"`

        // time constant for decay of dwnorm factor -- generally should be long-ish, between 1000-10000 -- integration rate factor is 1/tau
        float DecayTau; //  `min:"1" default:"1000,10000"`

        // minimum effective value of the normalization factor -- provides a lower bound to how much normalization can be applied
        float NormMin; //  `min:"0" default:"0.001"`

        // overall learning rate multiplier to compensate for changes due to use of normalization -- allows for a common master learning rate to be used between different conditions -- 0.1 for synapse-level, maybe higher for other levels
        float LrComp; //  `min:"0" default:"0.15"`

        // record the avg, max values of err, bcm hebbian, and overall dwt change per con group and per pathway
        bool Stats; //  `default:"false"`

        // rate constant of decay = 1 / decay_tau
        float DecayDt;  

        // complement rate constant of decay = 1 - (1 / decay_tau)
        float DecayDtC;

        DWtNormParams(bool on = true, float decayTau = 1000, float lrComp = 0.15, float normMin = 0.001, bool stats = false):
            On(on), DecayTau(decayTau), LrComp(lrComp), NormMin(normMin), Stats(stats){Update();};

        void Update();
        void Defaults(){On = true; DecayTau = 1000; LrComp = 0.15; NormMin = 0.001; Stats = false; Update();};
        float NormFromAbsDwt(float &norm, float absDwt);
    };

    // MomentumParams implements standard simple momentum -- accentuates consistent directions of weight change and
    // cancels out dithering -- biologically captures slower timecourse of longer-term plasticity mechanisms.
    struct MomentumParams{
        // whether to use standard simple momentum
        bool On; // bool `default:"true"`

        // time constant factor for integration of momentum -- 1/tau is dt (e.g., .1), and 1-1/tau (e.g., .95 or .9) is traditional momentum time-integration factor
        float MTau; // `min:"1" default:"10"`

        // overall learning rate multiplier to compensate for changes due to JUST momentum without normalization -- allows for a common master learning rate to be used between different conditions -- generally should use .1 to compensate for just momentum itself
        float LrComp; // `min:"0" default:"0.1"`

        // rate constant of momentum integration = 1 / m_tau
        float MDt;

        // complement rate constant of momentum integration = 1 - (1 / m_tau)
        float MDtC;

        MomentumParams(bool on = true, float mTau = 10, float lrComp = 0.1):
            On(on), MTau(mTau), LrComp(lrComp){Update();};

        void Update(){MDt = 1/MTau; MDtC = 1 - MDt;};
        void Defaults(){On = true, MTau = 10; LrComp = 0.1; Update();};
        float MomentumFromDWt(float &moment, float dwt);
    };

    // WtBalParams are weight balance soft renormalization params:
    // maintains overall weight balance by progressively penalizing weight increases as a function of
    // how strong the weights are overall (subject to thresholding) and long time-averaged activation.
    // Plugs into soft bounding function.
    struct WtBalParams{
        // perform weight balance soft normalization?  if so, maintains overall weight balance across units by progressively penalizing weight increases as a function of amount of averaged receiver weight above a high threshold (hi_thr) and long time-average activation above an act_thr -- this is generally very beneficial for larger models where hog units are a problem, but not as much for smaller models where the additional constraints are not beneficial -- uses a sigmoidal function: WbInc = 1 / (1 + HiGain*(WbAvg - HiThr) + ActGain * (nrn.ActAvg - ActThr)))
        bool On;

        // apply soft bounding to target layers -- appears to be beneficial but still testing
        bool Targs;

        // threshold on weight value for inclusion into the weight average that is then subject to the further HiThr threshold for then driving a change in weight balance -- this AvgThr allows only stronger weights to contribute so that weakening of lower weights does not dilute sensitivity to number and strength of strong weights
        float AvgThr; // `default:"0.25"`

        // high threshold on weight average (subject to AvgThr) before it drives changes in weight increase vs. decrease factors
        float HiThr; // `default:"0.4"`

        // gain multiplier applied to above-HiThr thresholded weight averages -- higher values turn weight increases down more rapidly as the weights become more imbalanced
        float HiGain; // `default:"4"`

        // low threshold on weight average (subject to AvgThr) before it drives changes in weight increase vs. decrease factors
        float LoThr; // `default:"0.4"`

        // gain multiplier applied to below-lo_thr thresholded weight averages -- higher values turn weight increases up more rapidly as the weights become more imbalanced -- generally beneficial but sometimes not -- worth experimenting with either 6 or 0
        float LoGain; // `default:"6,0"`

        WtBalParams(float on = true, bool targs = false, float avgThr = 0.25, float hiThr = 0.4, float hiGain = 4, float loThr = 0.4, float loGain = 6):
            On(on), Targs(targs), AvgThr(avgThr), HiThr(hiThr), HiGain(hiGain), LoThr(loThr), LoGain(loGain){};

        void Update(){};
        void Defaults(){On = false; AvgThr = 0.25; HiThr = 0.4; HiGain = 4; LoThr = 0.4; LoGain = 6;};
        std::tuple<float, float, float> WtBal(float wbAvg);
    };

    // leabra.LearnSynParams manages learning-related parameters at the synapse-level.
    struct LearnSynParams{
        // enable learning for this pathway
        bool Learn;

        // current effective learning rate (multiplies DWt values, determining rate of change of weights)
        float Lrate;

        // initial learning rate -- this is set from Lrate in UpdateParams, which is called when Params are updated, and used in LrateMult to compute a new learning rate for learning rate schedules.
        float LrateInit;

        // parameters for the XCal learning rule
        XCalParams XCal;

        // parameters for the sigmoidal contrast weight enhancement
        WtSigParams WtSig;

        // parameters for normalizing weight changes by abs max dwt
        DWtNormParams Norm;

        // parameters for momentum across weight changes
        MomentumParams Momentum;

        // parameters for balancing strength of weight increases vs. decreases
        WtBalParams WtBal;

        LearnSynParams(bool learn = true, float lrate = 0.04): Learn(learn), Lrate(lrate), LrateInit(lrate), XCal(), WtSig(), Norm(), Momentum(), WtBal() {};

        void Update();
        void Defaults();
        void LWtFromWt(Synapse& syn);
        void WtFromLWt(Synapse& syn);
        std::tuple<float, float> CHLdWt(float suAvgSLrn, float suAvgM, float ruAvgSLrn, float ruAvgM, float ruAvgL);
        float BCMdWt(float suAvgSLrn, float ruAvgSLrn, float ruAvgL);
        void WtFromDWt(float wbInc, float wbDec, float &dwt, float &wt, float &lwt, float scale);
    };

}