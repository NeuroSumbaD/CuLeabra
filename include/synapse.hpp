#pragma once
#include <string>
#include <vector>
#include <map>
#include "params.hpp"

namespace leabra {

    extern const std::vector<std::string> SynapseVars;//({"Wt", "LWt", "DWt", "Norm", "Moment", "Scale"});

    
    // leabra::Synapse holds state for the synaptic connection between neurons
    struct Synapse: params::StylerObject {
        float Wt;
        float LWt;
        float DWt;
        float Norm;
        float Moment;
        float Scale;

        float* SynapseVarByName(std::string varNm);
        float* VarByName(std::string varNm);
        void SetVarByName(std::string varNm, float val);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();
    };
    
} // namespace leabra
