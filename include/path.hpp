#pragma once
#include <fstream>
#include <functional>
#include "leabra.hpp"
#include "layer.hpp"
#include "act.hpp"
#include "synapse.hpp"
#include "tensor.hpp"

namespace paths {


    // Pattern defines a pattern of connectivity between two layers.
    // The pattern is stored efficiently using a bitslice tensor of binary values indicating
    // presence or absence of connection between two items.
    // A receiver-based organization is generally assumed but connectivity can go either way.
    struct Pattern {
        // Name returns the name of the pattern -- i.e., the "type" name of the actual pattern generatop
        virtual std::string Name();

        // Connect connects layers with the given shapes, returning the pattern of connectivity
        // as a bits tensor with shape = recv + send shapes, using row-major ordering with outer-most
        // indexes first (i.e., for each recv unit, there is a full inner-level of sender bits).
        // The number of connections for each recv and each send unit are also returned in
        // recvn and send tensors, each the shape of send and recv respectively.
        // The same flag should be set to true if the send and recv layers are the same (i.e., a self-connection)
        // often there are some different options for such connections.
        virtual std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> Connect(tensor::Shape &send, tensor::Shape &recv, bool same);
    };

    std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> NewTensors(tensor::Shape &send, tensor::Shape &recv);
    
    // Full implements full all-to-all pattern of connectivity between two layers
    struct Full: Pattern {
        // if true, and connecting layer to itself (self pathway), then make a self-connection from unit to itself
        bool SelfCon;

        std::string Name(){return "Full";};
        std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> Connect(tensor::Shape &send, tensor::Shape &recv, bool same);
    };
    
} // namespace paths


namespace leabra {

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
        Path(std::string name = "", std::string cls=""):emer::Path(name, cls){Send=nullptr; Recv=nullptr;};

        void UpdateParams();
        void Defaults();
        int NumSyns(){return Syns.size();};
        
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
        void SetScalesFunc(std::function<float(int si, int ri, tensor::Shape& send, tensor::Shape& recv)> wtFun);
        void InitWeightsSyn(Synapse& syn);
        void InitWeights();
        void InitWtSym(Synapse& syn);
        void InitGInc();
        void SendGDelta(int si, float delta);
        void RecvGInc();
        // Learn
        void DWt();
        void WtFromDWt();
        void WtBalFromWt();
        void LrateMult(float mult);
        
    };

    // WtBalRecvPath are state variables used in computing the WtBal weight balance function
    // There is one of these for each Recv Neuron participating in the pathway.
    struct WtBalRecvPath {
        // average of effective weight values that exceed WtBal.AvgThr across given Recv Neuron's connections for given Path
        float Avg;

        // overall weight balance factor that drives changes in WbInc vs. WbDec via a sigmoidal function -- this is the net strength of weight balance changes
        float Fact;

        // weight balance increment factor -- extra multiplier to add to weight increases to maintain overall weight balance
        float Inc;

        // weight balance decrement factor -- extra multiplier to add to weight decreases to maintain overall weight balance
        float Dec;

        WtBalRecvPath(){Avg = 0; Fact = 0; Inc = 1; Dec = 1;};
    };
    
} // namespace leabra

