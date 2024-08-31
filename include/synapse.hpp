#pragma once
#include <string>
#include <vector>
#include <map>

namespace leabra {

    std::vector<std::string> SynapseVars({"Wt", "LWt", "DWt", "Norm", "Moment", "Scale"});

    
    // leabra::Synapse holds state for the synaptic connection between neurons
    struct Synapse {
        float Wt;
        float LWt;
        float DWt;
        float Norm;
        float Moment;
        float Scale;

        float* SynapseVarByName(std::string varNm);
        float* VarByName(std::string varNm){return SynapseVarByName(varNm);};
        void SetVarByName(std::string varNm, float val){float *var = VarByName(varNm); *var = val;}
    };
    
} // namespace leabra
