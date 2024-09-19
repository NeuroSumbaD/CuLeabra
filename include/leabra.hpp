#pragma once
#include <vector>
#include <fstream>
#include "emer.hpp"
#include "tensor.hpp"
#include "time.hpp"
#include "fffb.hpp"
#include "params.hpp"

namespace leabra {

    // SelfInhibParams defines parameters for Neuron self-inhibition -- activation of the neuron directly feeds back
    // to produce a proportional additional contribution to Gi
    struct SelfInhibParams: params::StylerObject {
        bool On; // enable neuron self-inhibition
        float Gi; // [def: 0.4] strength of individual neuron self feedback inhibition -- can produce proportional activation behavior in individual units for specialized cases (e.g., scalar val or BG units), but not so good for typical hidden layers
        float Tau; // [def: 1.4] time constant in cycles, which should be milliseconds typically (roughly, how long it takes for value to change significantly -- 1.4x the half-life) for integrating unit self feedback inhibitory values -- prevents oscillations that otherwise occur -- relatively rapid 1.4 typically works, but may need to go longer if oscillations are a problem
        float Dt; // rate = 1 / tau

        SelfInhibParams(bool on=false, float gi=0.4, float tau=1.4):On(on),Gi(gi),Tau(tau){Update();};
        
        void Inhib(float &self, float act);
        void Update(){Dt = 1/Tau;};
        void Defaults(){On=false; Gi=0.4; Tau=1.4; Update();};

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };

    // ActAvgParams represents expected average activity levels in the layer.
    // Used for computing running-average computation that is then used for netinput scaling.
    // Also specifies time constant for updating average
    // and for the target value for adapting inhibition in inhib_adapt.
    struct ActAvgParams: params::StylerObject {
        float Init;
        bool Fixed;
        bool UseExtAct;
        bool UseFirst;
        float Tau;
        float Adjust;
        float Dt;

        ActAvgParams(float init=0.15, bool fixed=false, bool useExtAct=false, bool useFirst=true, float tau=100, float adjust=1):Init(init),Fixed(fixed),UseExtAct(useExtAct),UseFirst(useFirst),Tau(tau),Adjust(adjust){Update();};

        float EffInit();
        void AvgFromAct(float &avg, float act);
        void EffFromAvg(float &eff, float avg);

        void Update(){Dt = 1/Tau;};
        void Defaults(){Init=0.15, Fixed=false; UseExtAct=false; UseFirst=true; Tau=100; Adjust=1; Update();};

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };

    // leabra.InhibParams contains all the inhibition computation params and functions for basic Leabra
    // This is included in leabra.Layer to support computation.
    // This also includes other misc layer-level params such as running-average activation in the layer
    // which is used for netinput rescaling and potentially for adapting inhibition over time
    struct InhibParams: params::StylerObject {
        fffb::Params Layer; // inhibition across the entire layer
        fffb::Params Pool; // inhibition across sub-pools of units, for layers with 4D shape
        SelfInhibParams Self; // neuron self-inhibition parameters -- can be beneficial for producing more graded, linear response -- not typically used in cortical networks
        ActAvgParams ActAvg; // running-average activation computation values -- for overall estimates of layer activation levels, used in netinput scaling

        InhibParams():Layer(),Pool(),Self(),ActAvg(){};

        void Update();
        void Defaults();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };

    enum PathTypes {
        // Forward is a feedforward, bottom-up pathway from sensory inputs to higher layers
        ForwardPath,

        // Back is a feedback, top-down pathway from higher layers back to lower layers
        BackPath,

        // Lateral is a lateral pathway within the same layer / area
        LateralPath,

        // Inhib is an inhibitory pathway that drives inhibitory
        // synaptic conductances instead of the default excitatory ones.
        InhibPath,

        // CTCtxt are pathways from Superficial layers to CT layers that
        // send Burst activations drive updating of CtxtGe excitatory conductance,
        // at end of plus (51B Bursting) phase.  Biologically, this pathway
        // comes from the PT layer 5IB neurons, but it is simpler to use the
        // Super neurons directly, and PT are optional for most network types.
        // These pathways also use a special learning rule that
        // takes into account the temporal delays in the activation states.
        // Can also add self context from CT for deeper temporal context.
        CTCtxtPath
    };
    
    // WtBalRecvPath are state variables used in computing the WtBal weight balance function
    // There is one of these for each Recv Neuron participating in the pathway.
    struct WtBalRecvPath: params::StylerObject {
        // average of effective weight values that exceed WtBal.AvgThr across given Recv Neuron's connections for given Path
        float Avg;

        // overall weight balance factor that drives changes in WbInc vs. WbDec via a sigmoidal function -- this is the net strength of weight balance changes
        float Fact;

        // weight balance increment factor -- extra multiplier to add to weight increases to maintain overall weight balance
        float Inc;

        // weight balance decrement factor -- extra multiplier to add to weight decreases to maintain overall weight balance
        float Dec;

        WtBalRecvPath(){Avg = 0; Fact = 0; Inc = 1; Dec = 1;};

        void Init(){Avg = 0;Fact = 0;Inc = 1;Dec = 1;};

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };

    enum LayerTypes{
        // Super is a superficial cortical layer (lamina 2-3-4)
        // which does not receive direct input or targets.
        // In more generic models, it should be used as a Hidden layer,
        // and maps onto the Hidden type in LayerTypes.
        SuperLayer, 

        // Input is a layer that receives direct external input
        // in its Ext inputs.  Biologically, it can be a primary
        // sensory layer, or a thalamic layer.
        InputLayer,

        // Target is a layer that receives direct external target inputs
        // used for driving plus-phase learning.
        // Simple target layers are generally not used in more biological
        // models, which instead use predictive learning via Pulvinar
        // or related mechanisms.
        TargetLayer,

        // Compare is a layer that receives external comparison inputs,
        // which drive statistics but do NOT drive activation
        // or learning directly.  It is rarely used in axon.
        CompareLayer,


        // Deep

        // CT are layer 6 corticothalamic projecting neurons,
        // which drive "top down" predictions in Pulvinar layers.
        // They maintain information over time via stronger NMDA
        // channels and use maintained prior state information to
        // generate predictions about current states forming on Super
        // layers that then drive PT (5IB) bursting activity, which
        // are the plus-phase drivers of Pulvinar activity.
        CTLayer,

        // Pulvinar are thalamic relay cell neurons in the higher-order
        // Pulvinar nucleus of the thalamus, and functionally isomorphic
        // neurons in the MD thalamus, and potentially other areas.
        // These cells alternately reflect predictions driven by CT pathways,
        // and actual outcomes driven by 5IB Burst activity from corresponding
        // PT or Super layer neurons that provide strong driving inputs.
        PulvinarLayer,

        // TRNLayer is thalamic reticular nucleus layer for inhibitory competition
        // within the thalamus.
        TRNLayer,

        // PTMaintLayer implements the subset of pyramidal tract (PT)
        // layer 5 intrinsic bursting (5IB) deep neurons that exhibit
        // robust, stable maintenance of activity over the duration of a
        // goal engaged window, modulated by basal ganglia (BG) disinhibitory
        // gating, supported by strong MaintNMDA channels and recurrent excitation.
        // The lateral PTSelfMaint pathway uses MaintG to drive GMaintRaw input
        // that feeds into the stronger, longer MaintNMDA channels,
        // and the ThalToPT ModulatoryG pathway from BGThalamus multiplicatively
        // modulates the strength of other inputs, such that only at the time of
        // BG gating are these strong enough to drive sustained active maintenance.
        // Use Act.Dend.ModGain to parameterize.
        PTMaintLayer,

        // PTPredLayer implements the subset of pyramidal tract (PT)
        // layer 5 intrinsic bursting (5IB) deep neurons that combine
        // modulatory input from PTMaintLayer sustained maintenance and
        // CTLayer dynamic predictive learning that helps to predict
        // state changes during the period of active goal maintenance.
        // This layer provides the primary input to VSPatch US-timing
        // prediction layers, and other layers that require predictive dynamic
        PTPredLayer
    };

    struct Layer;
    struct Path: emer::Path {
        // sending layer for this pathway.
        Layer* Send;

        // receiving layer for this pathway.
        Layer* Recv;

        // type of pathway.
        PathTypes Type;

        // initial random weight distribution
        WtInitParams WtInit;

        // weight scaling parameters: modulates overall strength of pathway,
        // using both absolute and relative factors.
        WtScaleParams WtScale;

        // synaptic-level learning parameters
        LearnSynParams Learn;

        // synaptic state values, ordered by the sending layer
        // units which owns them -- one-to-one with SConIndex array.
        std::vector<Synapse*> Syns;

        // scaling factor for integrating synaptic input conductances (G's).
        // computed in AlphaCycInit, incorporates running-average activity levels.
        float GScale;

        // local per-recv unit increment accumulator for synaptic
        // conductance from sending units. goes to either GeRaw or GiRaw
        // on neuron depending on pathway type.
        std::vector<float>  GInc;

        // weight balance state variables for this pathway, one per recv neuron.
        std::vector<WtBalRecvPath> WbRecv;

        // number of recv connections for each neuron in the receiving layer,
        // as a flat list.
        std::vector<int> RConN;

        // average and maximum number of recv connections in the receiving layer.
        minmax::AvgMax32 RConNAvgMax;

        // starting index into ConIndex list for each neuron in
        // receiving layer; list incremented by ConN.
        std::vector<int> RConIndexSt;

        // index of other neuron on sending side of pathway,
        // ordered by the receiving layer's order of units as the
        // outer loop (each start is in ConIndexSt),
        // and then by the sending layer's units within that.
        std::vector<int> RConIndex;

        // index of synaptic state values for each recv unit x connection,
        // for the receiver pathway which does not own the synapses,
        // and instead indexes into sender-ordered list.
        std::vector<int> RSynIndex;

        // number of sending connections for each neuron in the
        // sending layer, as a flat list.
        std::vector<int> SConN;

        // average and maximum number of sending connections
        // in the sending layer.
        minmax::AvgMax32 SConNAvgMax;

        // starting index into ConIndex list for each neuron in
        // sending layer; list incremented by ConN.
        std::vector<int> SConIndexSt;

        // index of other neuron on receiving side of pathway,
        // ordered by the sending layer's order of units as the
        // outer loop (each start is in ConIndexSt), and then
        // by the sending layer's units within that.
        std::vector<int> SConIndex;

        // TODO:: FINISH initializer
        Path(std::string name = "", std::string cls="");

        void UpdateParams();
        void Defaults();
        int NumSyns();
        
        // maybe optional...
        int SynIndex(int sidx, int ridx);
        // float SynValue(std::string varNm, int sidx, int ridx);
        // void SetSynValue(std::string varNm, int sidx, int ridx, float val);

        // void WriteWeightsJSON(std::string fileName, int depth);
        // void WriteWeightsJSON(std::ofstream file, int depth);

        // void SetWeights(std::string fileName);
        // void SetWeights(std::ifstream file);

        void Connect(Layer* slay, Layer* rlay, paths::Pattern *pat, PathTypes typ);
        // void Validate(bool logmsg);
        void Build();
        int SetNIndexSt(std::vector<int> &n, minmax::AvgMax32 &avgmax, std::vector<int> &idxst, tensor::Int32 &tn);
        std::string String();

        void SetScalesRPool(tensor::Tensor<float> scales);
        void SetWtsFunc(std::function<float(int si, int ri, tensor::Shape& send, tensor::Shape& recv)> wtFun);
        void SetScalesFunc(std::function<float(int si, int ri, tensor::Shape& send, tensor::Shape& recv)> scaleFun);
        void InitWeightsSyn(Synapse& syn);
        void InitWeights();
        void InitWtSym(Path &rpt);
        void InitGInc();
        void SendGDelta(int si, float delta);
        void RecvGInc();
        // Learn
        void DWt();
        void WtFromDWt();
        void WtBalFromWt();
        void LrateMult(float mult);
        
        std::string TypeName();
        emer::Layer* SendLayer();
        emer::Layer* RecvLayer();

        void InitParamMaps();
    };

} // namespace leabra
