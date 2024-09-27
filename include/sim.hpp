/*
    The sim interface will coordinate all the tasks needed to run
    simulations with Leabra-style networks
*/
#pragma once
#include "pattable.hpp"
#include "tensor.hpp"
#include "params.hpp"

namespace leabra {

    // Environment provides an interface for generating external inputs to the
    // network. In the future I will implement some interface for converting a
    // python generator to an environment.
    struct Environment {
        float StepTime = 0; // number of milliseconds

        virtual void Init() = 0; // called at start of new epoch, resets counters
        virtual void Step() = 0; // incremented after each alpha cycle, prepares next set of patterns
        // TODO: Make step accept some set of mapped tensors with output from network
        virtual tensor::Tensor<float> *GetLayerInput(std::string name) = 0; // Gets the input to the layer of that name
        virtual bool EndEpoch() = 0; // Checks if the environment has reached the end of an epoch of training
        virtual int NumTrials() = 0; // Returns the maximum number of trials per epoch
    };

    struct TabulatedEnv: Environment {
        pattable::Table *table = nullptr;

        std::vector<int> permutation;
        uint EventIndex = 0;
        uint numEvents = 0;
        bool shuffle = false;

        TabulatedEnv();
        TabulatedEnv(std::string fileName);

        void Init() override;
        void Step() override;
        tensor::Tensor<float> *GetLayerInput(std::string layerName) override;
        bool EndEpoch() override;
        int NumTrials() override;

        void TableFromFile(std::string fileName);
    };
    
    struct Network;
    struct Context;

    struct Sim {
        Network *Net;
        Context *Ctx;
        params::Sets *Params;
        Environment *Env;

        std::map<std::string, std::vector<float>> EpochSSE; // map of target layer names and their SSE over each epoch
        std::map<std::string, std::vector<float>> TrialSSE; // map of target layer names and their SSE for each trial
        

        Sim(Network *net = nullptr, params::Sets *params = nullptr, Environment *env = nullptr);

        void Init();
        void AlphaCyc(bool train);
        void ApplyInputs();
        void TrainTrial();
        void StepTrial(bool train = true);
        void StepEpoch(bool train = true);
        void NewRun();
        void ApplyParams();

        void Run(int numEpochs, bool train = true);

        void RecordSSE(); // populates SSEmap with sse from each layer of type "TargetLayer"
    };
    
    
} // namespace leabra
