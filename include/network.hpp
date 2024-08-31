#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "tensor.hpp"
#include "layer.hpp"

namespace leabra {
    struct Network {
        std::vector<Layer*> Layers;
        std::map<std::string, Layer*> LayerMap;
        int NThreads;
        int WtBalInterval; // how frequently to update the weight balance average weight factor -- relatively expensive.
        int WtBalCtr; // counter for how long it has been since last WtBal.

        std::string Name;

        Network(std::string name, int wtBalInterval = 10):Name(name),WtBalInterval(wtBalInterval){NThreads = 1;WtBalCtr = 0;};

        int NumLayers(){return Layers.size();};
        Layer* EmerLayer(int idx) {return Layers[idx];};
        int MaxParallelData(){return 1;};
        int NParallelData(){return 1;};

        void Defaults();
        void UpdateParams();
        void AddLayerInit(Layer* ly, std::string name, LayerShape shape, LayerTypes typ);
        void AddLayer(std::string name, LayerShape shape, LayerTypes typ);
        void AddLayer2D(std::string name, int shapeY, int shapeX, LayerTypes typ);
        void AddLayer4D(std::string name, int nPoolsY, int nPoolsX, int shapeY, int shapeX, LayerTypes typ);
        std::tuple<Layer*, Layer*, Path*> ConnectLayerNames(std::string send, std::string recv, paths::Pattern pat, PathTypes typ);
        Path* ConnectLayers(Layer* send, Layer* recv, paths::Pattern pat, PathTypes typ);
        std::tuple<Layer*, Layer*, Path*> BidirConnectLayerNames(std::string send, std::string recv, paths::Pattern pat, PathTypes typ);
        Path* BidirConnectLayers(Layer* send, Layer* recv, paths::Pattern pat, PathTypes typ);
        Path* LateralConnectLayers(Layer* lay, paths::Pattern pat);
        Path* LateralConnectLayersPath(Layer* lay, paths::Pattern pat, Path* pt);
        void Build();
        // std::tuple<int,int> VarRange(std::string varName); // VarRange returns the min / max values for given variable

        void AlphaCycInit(bool updActAvg);
        void Cycle(Context* ctx);
        // Act methods
        void SendGDelta(Context* ctx);
        void AvgMaxGe(Context* ctx);
        void InhibFromGeAct(Context* ctx);
        void ActFromG(Context* ctx);
        void AvgMaxAct(Context* ctx);
        void QuarterFinal(Context* ctx);
        void MinusPhase(Context* ctx);
        void PlusPhase(Context* ctx);
        // Learn Methods
        void Dwt();
        void WtFromDwt();
        void LrateMult(float mult);
        // Init Methods
        void InitWeights();
        void InitTopoScales();
        void DecayState(float decay);
        void InitActs();
        void InitExt();
        void UpdateExtFlags();
        void InitGInc();
        void GScaleFromAvgAct();
        // Lesion Methods
        void LayersSetOff(bool off);
        void UnLesionNeurons();
        
    };
    
} // namespace leabra
