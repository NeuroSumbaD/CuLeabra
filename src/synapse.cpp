#include "synapse.hpp"
#include <iostream>

namespace leabra {
    const std::vector<std::string> SynapseVars({"Wt", "LWt", "DWt", "Norm", "Moment", "Scale"});
} // namespace leabra


// SynapseVarByName returns a pointer to the variable in the Synapse, or error
float* leabra::Synapse::SynapseVarByName(std::string varNm) {
    if (varNm == "Wt") {
        return &Wt;
    } else if (varNm == "LWt") {
        return &LWt;
    } else if (varNm == "DWt") {
        return &DWt;
    } else if (varNm == "Norm") {
        return &Norm;
    } else if (varNm == "Moment") {
        return &Moment;
    } else if (varNm == "Scale") {
        return &Scale;
    } else {
        throw std::runtime_error("Synapse does not have variable named: " + varNm);
    }
}

float *leabra::Synapse::VarByName(std::string varNm) {
    return SynapseVarByName(varNm);
}

void leabra::Synapse::SetVarByName(std::string varNm, float val) {
    float *var = VarByName(varNm);
    *var = val;
}

std::string leabra::Synapse::StyleType() {
    return "Synapse";
}

std::string leabra::Synapse::StyleClass() {
    return "";
}

std::string leabra::Synapse::StyleName() {
    return "";
}

void leabra::Synapse::InitParamMaps() {
    ParamNameMap["Scale"] = (void*) &Scale;

    ParamTypeMap["Scale"] = &typeid(float);
}
