#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "tensor.hpp"
#include "emer.hpp"
#include "leabra.hpp"
#include "context.hpp"

namespace leabra {
    struct Layer; //enum LayerTypes; enum PathTypes;
    struct Path;

    struct Network: emer::Network {
        std::vector<Layer*> Layers;
        std::map<std::string, Layer*> LayerMap;
        int NThreads;
        int WtBalInterval; // how frequently to update the weight balance average weight factor -- relatively expensive.
        int WtBalCtr; // counter for how long it has been since last WtBal.

        Network(std::string name, int wtBalInterval = 10);

        int NumLayers();
        emer::Layer* EmerLayer(int idx);
        int MaxParallelData();
        int NParallelData();

        void Defaults();
        void UpdateParams();
        void AddLayerInit(Layer* ly, std::string name, std::vector<int> shape, LayerTypes typ);
        Layer* AddLayer(std::string name, std::vector<int> shape, LayerTypes typ);
        Layer* AddLayer2D(std::string name, int shapeY, int shapeX, LayerTypes typ);
        Layer* AddLayer4D(std::string name, int nPoolsY, int nPoolsX, int nNeurY, int nNeurX, LayerTypes typ);
        std::tuple<Layer*, Layer*, Path*> ConnectLayerNames(std::string send, std::string recv, paths::Pattern *pat, PathTypes typ);
        Path* ConnectLayers(Layer* send, Layer* recv, paths::Pattern *pat, PathTypes typ);
        std::tuple<Layer*, Layer*, Path*, Path *> BidirConnectLayerNames(std::string low, std::string high, paths::Pattern *pat);
        std::tuple<leabra::Path *, leabra::Path *> BidirConnectLayers(Layer* low, Layer* high, paths::Pattern *pat);
        Path* LateralConnectLayer(Layer* lay, paths::Pattern *pat);
        Path* LateralConnectLayerPath(Layer* lay, paths::Pattern *pat, Path* pt);
        void Build();
        // std::tuple<int,int> VarRange(std::string varName); // VarRange returns the min / max values for given variable

        void AlphaCycInit(bool updtActAvg);
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
