#pragma once
#include <tuple>
#include "relpos.hpp"
#include "leabra.hpp"
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
    struct Network;

    struct LayerShape{
        int X;
        int Y;
        int PoolsX;
        int PoolsY;

        LayerShape(int x, int y, int poolsX=1, int poolsY = 1);
    };

    struct Layer: emer::Layer {
        int Index;
        Network* Net;// our parent network, in case we need to use it to find other layers etc; set when added by network.
        LayerTypes Type;
        std::vector<Path*> RecvPaths;
        std::vector<Path*> SendPaths;
        ActParams Act;
        InhibParams Inhib;
        LearnNeurParams Learn;
        std::vector<Neuron> Neurons;
        std::vector<Pool> Pools;
        CosDiffStats CosDiff;

        Layer(std::string name, int index = 0, Network* net = nullptr);
        

        void Defaults();
        void UpdateParams();
        Path* RecipToSendPath(Path * spj);
        Pool* GetPool(int idx); // Pool returns pointer to pool at given index

        //Build
        void BuildSubPools();
        void BuildPools(int nu);
        void BuildPaths();
        void Build();
        // void WriteWeightsJSON(std::ifstream jsonFile, int depth);
        // void SetWeights(weights::Layer lw);
        // std::tuple<int,int> VarRange(std::string varName); // VarRange returns the min / max values for given variable

        void InitWeights();
        void InitActAvg();
        void InitActs();
        void InitWtSym();
        void InitExt();

        std::tuple<std::vector<int>, std::vector<int>, bool> ApplyExtFlags();
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
        void InitGInc();
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
        void CyclePost(Context* ctx);
        // Quarter
        void QuarterFinal(Context* ctx);
        void MinusPhase(Context* ctx);
        void PlusPhase(Context* ctx);
        void CosDiffFromActs();
        bool IsTarget();
        // Learning
        void DWt();
        void WtFromDWt();
        void WtBalFromWt();
        void LrateMult(float mult);
        // Threading / Reports
        std::tuple<int, int, int> CostEst();
        // Stats
        std::tuple<int, int> MSE(float tol = 0.5);
        float SSE(float tol = 0.5);
        // Lesion
        void UnLesionNeurons();
        int LesionNeurons(float prop);

        // emer::Layer virtual methods
        // void *StyleObject();
        std::string TypeName();
        // int TypeNumber();
        int NumRecvPaths();
        emer::Path* RecvPath(int idx);
        int NumSendPaths();
        emer::Path* SendPath(int idx);

        void InitParamMaps();
    };

};