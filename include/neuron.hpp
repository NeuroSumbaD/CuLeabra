#pragma once

#include <vector>
#include "params.hpp"

namespace leabra {
    enum NeurFlags{ // NeurFlags are bit-flags encoding relevant binary state for neurons
        NeurOff, // NeurOff flag indicates that this neuron has been turned off (i.e., lesioned)
        NeurHasExt, // NeurHasExt means the neuron has external input in its Ext field
        NeurHasTarg, // NeurHasTarg means the neuron has external target input in its Targ field
        NeurHasCmpr, // NeurHasCmpr means the neuron has external comparison input in its Targ field -- used for computing comparison statistics but does not drive neural activity ever
        NeurFlagsN
    };

    // leabra.Neuron holds all of the neuron (unit) level variables -- this is the most basic version with
    // rate-code only and no optional features at all.
    // All variables accessible via Unit interface must be float32 and start at the top, in contiguous order
    struct Neuron: params::StylerObject {
        NeurFlags Flags;
        int SubPool;
        float Act;
        float ActLrn;
        float Ge;
        float Gi;
        float Gk;
        float Inet;
        float Vm;
        float Targ;
        float Ext;
        float AvgSS;
        float AvgS;
        float AvgM;
        float AvgL;
        float AvgLLrn;
        float AvgSLrn;
        float ActQ0;
        float ActQ1;
        float ActQ2;
        float ActQM;
        float ActM;
        float ActP;
        float ActDif;
        float ActDel;
        float ActAvg;
        float Noise;
        float GiSyn;
        float GiSelf;
        float ActSent;
        float GeRaw;
        float GiRaw;
        float GknaFast;
        float GknaMed;
        float GknaSlow;
        float Spike;
        float ISI;
        float ISIAvg;
        bool HasFlag(NeurFlags flag);
        void SetFlag(bool on, std::vector<int> flags);
        // void ClearFlag(NeurFlags flag);
        // void SetMask(int mask);
        // void ClearMask(int mask);
        bool IsOff();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~Neuron() = default;
    };
}