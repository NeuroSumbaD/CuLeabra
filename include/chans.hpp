#pragma once
#include "params.hpp"
/*
Package chans provides standard neural conductance channels for computing
a point-neuron approximation based on the standard equivalent RC circuit
model of a neuron (i.e., basic Ohms law equations).
Includes excitatory, leak, inhibition, and dynamic potassium channels.
*/

namespace chans {

    // Chans are ion channels used in computing point-neuron activation function
    struct Chans: params::StylerObject {
        float E; // excitatory sodium (Na) AMPA channels activated by synaptic glutamate
        float L; // constant leak (potassium, K+) channels -- determines resting potential (typically higher than resting potential of K)
        float I; // inhibitory chloride (Cl-) channels activated by synaptic GABA
        float K; // gated / active potassium channels -- typically hyperpolarizing relative to leak / rest

        Chans(float e, float l, float i, float k);
        void SetAll(float e, float l, float i, float k);
        void SetFromOtherMinus(Chans oth, float minus);
        void SetFromMinusOther(float minus, Chans oth);

        std::string StyleType();
        std::string StyleClass();
        std::string StyleName();

        void InitParamMaps();

        ~Chans() = default;
    };
    
}