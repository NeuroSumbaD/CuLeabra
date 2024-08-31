#pragma once
#include <vector>
#include <fstream>
#include "emer.hpp"
#include "tensor.hpp"
#include "time.hpp"
#include "fffb.hpp"

namespace leabra {

    // SelfInhibParams defines parameters for Neuron self-inhibition -- activation of the neuron directly feeds back
    // to produce a proportional additional contribution to Gi
    struct SelfInhibParams {
        bool On; // enable neuron self-inhibition
        float Gi; // [def: 0.4] strength of individual neuron self feedback inhibition -- can produce proportional activation behavior in individual units for specialized cases (e.g., scalar val or BG units), but not so good for typical hidden layers
        float Tau; // [def: 1.4] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life) for integrating unit self feedback inhibitory values -- prevents oscillations that otherwise occur -- relatively rapid 1.4 typically works, but may need to go longer if oscillations are a problem
        float Dt; // rate = 1 / tau

        SelfInhibParams(bool on=false, float gi=0.4, float tau=1.4):On(on),Gi(gi),Tau(tau){Update();};
        
        void Inhib(float &self, float act);
        void Update(){Dt = 1/Tau;};
        void Defaults(){On=false; Gi=0.4; Tau=1.4; Update();};
    };

    // ActAvgParams represents expected average activity levels in the layer.
    // Used for computing running-average computation that is then used for netinput scaling.
    // Also specifies time constant for updating average
    // and for the target value for adapting inhibition in inhib_adapt.
    struct ActAvgParams{
        float Init;
        bool Fixed;
        bool UseExtAct;
        bool UseFirst;
        float Tau;
        float Adjust;
        float Dt;

        ActAvgParams(float init=0.15, bool fixed=false, bool useExtAct=false, bool useFirst=true, float tau=100, float adjust=1):Init(init),Fixed(fixed),UseExtAct(useExtAct),UseFirst(useFirst),Tau(tau),Adjust(adjust){Update();};

        float EffInit();
        void AvgFmAct(float &avg, float act);
        void EffFmAvg(float &eff, float avg);

        void Update(){Dt = 1/Tau;};
        void Defaults(){Init=0.15, Fixed=false; UseExtAct=false; UseFirst=true; Tau=100; Adjust=1; Update();};
    };

    // leabra.InhibParams contains all the inhibition computation params and functions for basic Leabra
    // This is included in leabra.Layer to support computation.
    // This also includes other misc layer-level params such as running-average activation in the layer
    // which is used for netinput rescaling and potentially for adapting inhibition over time
    struct InhibParams{
        fffb::Params Layer; // inhibition across the entire layer
        fffb::Params Pool; // inhibition across sub-pools of units, for layers with 4D shape
        SelfInhibParams Self; // neuron self-inhibition parameters -- can be beneficial for producing more graded, linear response -- not typically used in cortical networks
        ActAvgParams ActAvg; // running-average activation computation values -- for overall estimates of layer activation levels, used in netinput scaling

        InhibParams():Layer(),Pool(),Self(),ActAvg(){};

        void Update();
        void Defaults();
    };

    // // LeabraNetwork defines the essential algorithmic API for Leabra, at the network level.
    // // These are the methods that the user calls in their Sim code:
    // // * AlphaCycInit
    // // * Cycle
    // // * QuarterFinal
    // // * DWt
    // // * WtFmDwt
    // // Because we don't want to have to force the user to use the interface cast in calling
    // // these methods, we provide Impl versions here that are the implementations
    // // which the user-facing method calls.
    // //
    // // Typically most changes in algorithm can be accomplished directly in the Layer
    // // or Prjn level, but sometimes (e.g., in deep) additional full-network passes
    // // are required.
    // //
    // // All of the structural API is in emer.Network, which this interface also inherits for
    // // convenience.
    // struct LeabraNetwork: emer::Network {
    //     // NewLayer creates a new concrete layer of appropriate type for this network
    //     virtual emer::Layer NewLayer();

    //     // NewPrjn creates a new concrete projection of appropriate type for this network
    //     virtual emer::Prjn NewPrjn();

    //     // AlphaCycInit handles all initialization at start of new input pattern.
    //     // Should already have presented the external input to the network at this point.
    //     // If updtActAvg is true, this includes updating the running-average
    //     // activations for each layer / pool, and the AvgL running average used
    //     // in BCM Hebbian learning.
    //     // The input scaling is updated  based on the layer-level running average acts,
    //     // and this can then change the behavior of the network,
    //     // so if you want 100% repeatable testing results, set this to false to
    //     // keep the existing scaling factors (e.g., can pass a train bool to
    //     // only update during training).  This flag also affects the AvgL learning
    //     // threshold
    //     virtual void AlphaCycInitImpl(bool updtActAvg);

    //     // CycleImpl runs one cycle of activation updating:
    //     // * Sends Ge increments from sending to receiving layers
    //     // * Average and Max Ge stats
    //     // * Inhibition based on Ge stats and Act Stats (computed at end of Cycle)
    //     // * Activation from Ge, Gi, and Gl
    //     // * Average and Max Act stats
    //     // This basic version doesn't use the time info, but more specialized types do, and we
    //     // want to keep a consistent API for end-user code.
    //     virtual void CycleImpl(time::Time& ltime);

    //     // CyclePostImpl is called after the standard Cycle update, and calls CyclePost
    //     // on Layers -- this is reserved for any kind of special ad-hoc types that
    //     // need to do something special after Act is finally computed.
    //     // For example, sending a neuromodulatory signal such as dopamine.
    //     virtual void CyclePostImpl(time::Time& ltime);

    //     // QuarterFinalImpl does updating after end of a quarter
    //     virtual void QuarterFinalImpl(time::Time& ltime);

    //     // DWtImpl computes the weight change (learning) based on current
    //     // running-average activation values
    //     virtual void DWtImpl();

    //     // WtFmDWtImpl updates the weights from delta-weight changes.
    //     // Also calls WtBalFmWt every WtBalInterval times
    //     virtual void WtFmDWtImpl();
    // };

    
    // struct LeabraLayer: emer::Layer {
    //     // SetThread sets the thread number for this layer to run on
    //     virtual void SetThread(int thr);

    //     // InitWts initializes the weight values in the network, i.e., resetting learning
    //     // Also calls InitActs
    //     virtual void InitWts();

    //     // InitActAvg initializes the running-average activation values that drive learning.
    //     virtual void InitActAvg();

    //     // InitActs fully initializes activation state -- only called automatically during InitWts
    //     virtual void InitActs();

    //     // InitWtsSym initializes the weight symmetry -- higher layers copy weights from lower layers
    //     virtual void InitWtSym();

    //     // InitExt initializes external input state -- called prior to apply ext
    //     virtual void InitExt();

    //     // ApplyExt applies external input in the form of an tensor.Tensor
    //     // If the layer is a Target or Compare layer type, then it goes in Targ
    //     // otherwise it goes in Ext.
    //     virtual void ApplyExt(tensor::Tensor ext);

    //     // ApplyExt1D applies external input in the form of a flat 1-dimensional slice of floats
    //     // If the layer is a Target or Compare layer type, then it goes in Targ
    //     // otherwise it goes in Ext
    //     virtual void ApplyExt1D(std::vector<float> ext);

    //     // UpdateExtFlags updates the neuron flags for external input based on current
    //     // layer Type field -- call this if the Type has changed since the last
    //     // ApplyExt* method call.
    //     virtual void UpdateExtFlags();

    //     // // RecvPrjns returns the slice of receiving projections for this layer
    //     // virtual LeabraPrjns* RecvPrjns();

    //     // // SendPrjns returns the slice of sending projections for this layer
    //     // virtual LeabraPrjns* SendPrjns();

    //     // IsTarget returns true if this layer is a Target layer.
    //     // By default, returns true for layers of Type == emer.Target
    //     // Other Target layers include the TRCLayer in deep predictive learning.
    //     // This is used for turning off BCM hebbian learning,
    //     // in CosDiffFmActs to set the CosDiff.ModAvgLLrn value
    //     // for error-modulated level of hebbian learning.
    //     // It is also used in WtBal to not apply it to target layers.
    //     // In both cases, Target layers are purely error-driven.
    //     virtual bool IsTarget();

    //     // AlphaCycInit handles all initialization at start of new input pattern.
    //     // Should already have presented the external input to the network at this point.
    //     // If updtActAvg is true, this includes updating the running-average
    //     // activations for each layer / pool, and the AvgL running average used
    //     // in BCM Hebbian learning.
    //     // The input scaling is updated  based on the layer-level running average acts,
    //     // and this can then change the behavior of the network,
    //     // so if you want 100% repeatable testing results, set this to false to
    //     // keep the existing scaling factors (e.g., can pass a train bool to
    //     // only update during training).  This flag also affects the AvgL learning
    //     // threshold
    //     virtual void AlphaCycInit(bool updtActAvg);

    //     // AvgLFmAvgM updates AvgL long-term running average activation that
    //     // drives BCM Hebbian learning
    //     virtual void AvgLFmAvgM();

    //     // GScaleFmAvgAct computes the scaling factor for synaptic conductance input
    //     // based on sending layer average activation.
    //     // This attempts to automatically adjust for overall differences in raw
    //     // activity coming into the units to achieve a general target
    //     // of around .5 to 1 for the integrated G values.
    //     virtual void GScaleFmAvgAct();

    //     // GenNoise generates random noise for all neurons
    //     virtual void GenNoise();

    //     // DecayState decays activation state by given proportion (default is on ly.Act.Init.Decay)
    //     virtual void DecayState(float decay);

    //     // HardClamp hard-clamps the activations in the layer -- called during AlphaCycInit
    //     // for hard-clamped Input layers
    //     virtual void HardClamp();

    //     //////////////////////////////////////////////////////////////////////////////////////
    //     //  Cycle Methods

    //     // InitGInc initializes synaptic conductance increments -- optional
    //     virtual void InitGInc();

    //     // SendGDelta sends change in activation since last sent, to increment recv
    //     // synaptic conductances G, if above thresholds
    //     virtual void SendGDelta(time::Time& ltime);

    //     // GFmInc integrates new synaptic conductances from increments sent during last SendGDelta
    //     virtual void GFmInc(time::Time& ltime);

    //     // AvgMaxGe computes the average and max Ge stats, used in inhibition
    //     virtual void AvgMaxGe(time::Time& ltime);

    //     // InhibiFmGeAct computes inhibition Gi from Ge and Act averages within relevant Pools
    //     virtual void InhibFmGeAct(time::Time& ltime);

    //     // ActFmG computes rate-code activation from Ge, Gi, Gl conductances
    //     // and updates learning running-average activations from that Act
    //     virtual void ActFmG(time::Time& ltime);

    //     // AvgMaxAct computes the average and max Act stats, used in inhibition
    //     virtual void AvgMaxAct(time::Time& ltime);

    //     // CyclePost is called after the standard Cycle update, as a separate
    //     // network layer loop.
    //     // This is reserved for any kind of special ad-hoc types that
    //     // need to do something special after Act is finally computed.
    //     // For example, sending a neuromodulatory signal such as dopamine.
    //     virtual void CyclePost(time::Time& ltime);

    //     //////////////////////////////////////////////////////////////////////////////////////
    //     //  Quarter Methods

    //     // QuarterFinal does updating after end of a quarter
    //     virtual void QuarterFinal(time::Time& ltime);

    //     // CosDiffFmActs computes the cosine difference in activation state
    //     // between minus and plus phases.
    //     // This is also used for modulating the amount of BCM hebbian learning
    //     virtual void CosDiffFmActs();

    //     // DWt computes the weight change (learning) -- calls DWt method on sending projections
    //     virtual void DWt();

    //     // WtFmDWt updates the weights from delta-weight changes -- on the sending projections
    //     virtual void WtFmDWt();

    //     // WtBalFmWt computes the Weight Balance factors based on average recv weights
    //     virtual void WtBalFmWt();

    //     // LrateMult sets the new Lrate parameter for Prjns to LrateInit * mult.
    //     // Useful for implementing learning rate schedules.
    //     virtual void LrateMult(float mult);
    // };

    // struct LeabraPrjn: emer::Prjn {
    //     // InitWts initializes weight values according to Learn.WtInit params
    //     virtual void InitWts();
        
    //     virtual void InitWtsSym();
        
    //     virtual void InitGInc();
        
    //     virtual void SendGDelta(int si, float delta);
        
    //     virtual void RecvGInc();
        
    //     virtual void DWt();
        
    //     virtual void WtFmDWt();
        
    //     virtual void WtBalFmWt();
        
    //     virtual void LrateMult(float mult);
    // };

    // typedef std::vector<LeabraPrjn> LeabraPrjns;
} // namespace leabra
