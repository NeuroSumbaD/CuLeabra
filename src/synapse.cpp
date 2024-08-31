#include "synapse.hpp"
#include <iostream>


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