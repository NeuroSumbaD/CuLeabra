#pragma once
#include <tuple>
#include "relpos.hpp"
// #include "emer.hpp"
#include "leabra.hpp"
#include "network.hpp"
#include "path.hpp"
#include "act.hpp"
#include "context.hpp"

#include "bitflag.hpp"
#include "tensor.hpp"

#include "learn.hpp"
#include "neuron.hpp"
#include "pool.hpp"
#include "act.hpp"

namespace leabra {
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

    struct LayerShape{
        int X;
        int Y;
        int PoolsX;
        int PoolsY;

        LayerShape(int x, int y, int poolsX=1, int poolsY = 1): X(x),Y(y),PoolsX(poolsX),PoolsY(poolsY){};
    };

    struct Layer: emer::Layer {
        int Index;
        Network* Net;// our parent network, in case we need to use it to find other layers etc; set when added by network.
        LayerTypes Type;
        std::vector<Path> RecvPaths;
        std::vector<Path> SendPaths;
        ActParams Act;
        InhibParams Inhib;
        LearnNeurParams Learn;
        std::vector<Neuron> Neurons;
        std::vector<Pool> Pools;
        CosDiffStats CosDiff;

        Layer(std::string name, int index = 0, Network* net = nullptr)
            :Index(index), Net(net), RecvPaths(), SendPaths(), Act(), Inhib(), Learn(), Neurons(), Pools(), CosDiff(), emer::Layer(name){Inhib.Layer.On = true;};
        

        void Defaults();
        void UpdateParams();
        Pool* GetPool(int idx){return &(Pools[idx]);}; // Pool returns pointer to pool at given index

        //Build
        // void BuildSubPools();
        // void BuildPools();
        // void BuildPaths();
        // void Build();
        // void WriteWeightsJSON(std::ifstream jsonFile, int depth);
        // void SetWeights(weights::Layer lw);
        // std::tuple<int,int> VarRange(std::string varName); // VarRange returns the min / max values for given variable

        void InitWeights();
        void InitActAvg();
        void InitActs();
        void InitWtSym();
        void InitExt();

        std::tuple<bool, bool, bool> ApplyExtFlags();
        void ApplyExt(tensor::Tensor<float> ext);
        void ApplyExt2D(tensor::Tensor<float> ext);
        void ApplyExt2Dto4D(tensor::Tensor<float> ext);
        void ApplyExt4D(tensor::Tensor<float> ext);
        void ApplyExt1DTsr(std::vector<float> ext);
        void ApplyExt1D(std::vector<float> ext);
        // void ApplyExt1D32(float ext);
        void UpdateExtFlags();

        void ActAvgFromAct();
        void ActQ0FromActP();
        void AlphaCycInit(bool updtActAvg);
        void AvgLFromAvgM();
        void GScaleFromAvgAct();
        void GenNoise();
        void DecayState(float decay);
        void DecayStatePool(int pool, float decay);
        void HardClamp();
        // Cycle
        void InitGinc();
        void SendGDelta(Context* ctx);
        void GFromInc(Context* ctx);
        void RecvGInc(Context* ctx);
        void GFromIncNeur(Context* ctx);
        void AvgMaxGe(Context* ctx);
        void InhibFromGeAct(Context* ctx);
        void PoolInhibFromGeAct(Context* ctx);
        void InhibFromPool(Context* ctx);
        void ActFromG(Context* ctx);
        void AvgMaxAct(Context* ctx);
        void CyclePost(Context* ctx){};
        // Quarter
        void QuarterFinal(Context* ctx);
        void MinusPhase(Context* ctx);
        void PlusPhase(Context* ctx);
        void CosDiffFromActs();
        bool IsTarget(){return Type==LayerTypes::TargetLayer;};
        // Learning
        void DWt();
        void WtFromDWt();
        void WtBalFromWt();
        void LrateMult(float mult);
        // Threading / Reports
        std::tuple<int, int, int> CostEst();
        // Stats
        std::tuple<int, int> MSE(float tol);
        float SSE(float tol);
        // Lesion
        void UnLesionNeurons();
        void LesionNeurons();
    };

};