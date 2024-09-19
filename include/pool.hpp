#pragma once
#include "fffb.hpp"
#include "minmax.hpp"

namespace leabra {

    // ActAvg are running-average activation levels used for netinput scaling and adaptive inhibition
    struct ActAvg {
        float ActMAvg; // running-average minus-phase activity -- used for adapting inhibition -- see ActAvgParams.Tau for time constant etc
        float ActPAvg; // running-average plus-phase activity -- used for synaptic input scaling -- see ActAvgParams.Tau for time constant etc
        float ActPAvgEff; // ActPAvg * ActAvgParams.Adjust -- adjusted effective layer activity directly used in synaptic input scaling
    };

    // Pool contains computed values for FFFB inhibition, and various other state values for layers
    // and pools (unit groups) that can be subject to inhibition, including:
    // * average / max stats on Ge and Act that drive inhibition
    // * average activity overall that is used for normalizing netin (at layer level)
    struct Pool: params::StylerObject {
        int StIndex,
            EdIndex; // starting and ending (exlusive) indexes for the list of neurons in this pool
        fffb::Inhib Inhib; // FFFB inhibition computed values, including Ge and Act AvgMax which drive inhibition
        minmax::AvgMax32 ActM; // minus phase average and max Act activation values, for ActAvg updt
        minmax::AvgMax32 ActP; // plus phase average and max Act activation values, for ActAvg updt
        ActAvg ActAvgs; // running-average activation levels used for netinput scaling and adaptive inhibition

        Pool();
        void Init();

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };
    
} // namespace leabra
