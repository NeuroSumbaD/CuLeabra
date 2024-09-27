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
        int SubPool = 0;
        float Act = 0;
        float ActLrn = 0;
        float Ge = 0;
        float Gi = 0;
        float Gk = 0;
        float Inet = 0;
        float Vm = 0;
        float Targ = 0;
        float Ext = 0;
        float AvgSS = 0;
        float AvgS = 0;
        float AvgM = 0;
        float AvgL = 0;
        float AvgLLrn = 0;
        float AvgSLrn = 0;
        float ActQ0 = 0;
        float ActQ1 = 0;
        float ActQ2 = 0;
        float ActQM = 0;
        float ActM = 0;
        float ActP = 0;
        float ActDif = 0;
        float ActDel = 0;
        float ActAvg = 0;
        float Noise = 0;
        float GiSyn = 0;
        float GiSelf = 0;
        float ActSent = 0;
        float GeRaw = 0;
        float GiRaw = 0;
        float GknaFast = 0;
        float GknaMed = 0;
        float GknaSlow = 0;
        float Spike = 0;
        float ISI = 0;
        float ISIAvg = 0;

        Neuron();
        
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