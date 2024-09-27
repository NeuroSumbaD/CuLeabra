#include "sim.hpp"
#include "rand.hpp"

#include "leabra.hpp"
#include "network.hpp"
#include "layer.hpp"

void leabra::Sim::Init() {
    Net->SetRandSeed(0);

    Net->Build();
	Net->Defaults();
	Net->InitWeights();

    ApplyParams(); // possibly redundant??
    NewRun();

    EpochSSE = std::map<std::string, std::vector<float>>();
    TrialSSE = std::map<std::string, std::vector<float>>();
    for (Layer *lay: Net->Layers){
        if (lay->Type == TargetLayer) {
            EpochSSE[lay->Name] = std::vector<float>();
            TrialSSE[lay->Name] = std::vector<float>();
            TrialSSE[lay->Name].reserve(Env->NumTrials());
        }
    }
}

// AlphaCyc runs one alpha-cycle (100 msec, 4 quarters)			 of processing.
// External inputs must have already been applied prior to calling,
// using ApplyExt method on relevant layers (see TrainTrial, TestTrial).
// If train is true, then learning DWt or WtFmDWt calls are made.
// Handles netview updating within scope of AlphaCycle
void leabra::Sim::AlphaCyc(bool train) {
	Net->AlphaCycInit(train);
	Ctx->AlphaCycStart();
	for (int qtr = 0; qtr < 4; qtr++) {
		for (int cyc = 0; cyc < Ctx->CycPerQtr; cyc++) {
			Net->Cycle(Ctx);
			Ctx->CycleInc();
		}
		Net->QuarterFinal(Ctx);
		Ctx->QuarterInc();
	}

	if (train) {
		Net->Dwt();
		Net->WtFromDwt();
	}
}

// ApplyInputs applies input patterns from given envirbonment.
// It is good practice to have this be a separate method with appropriate
// args so that it can be used for various different contexts
// (training, testing, etc).
void leabra::Sim::ApplyInputs() {
    // ss.Net.InitExt() // clear any existing inputs -- not strictly necessary if always
	// going to the same layers, but good practice and cheap anyway

	std::vector<std::string> lays = {"Input", "Output"};
	for (std::string &lnm: lays) {
		leabra::Layer *ly = dynamic_cast<leabra::Layer*>(Net->LayerByName(lnm));
        // TODO: insert some check here to make sure it correctly returns something
		// pats := en.State(ly.Nm)
        tensor::Tensor<float> *pats = Env->GetLayerInput(ly->Name);
		if (pats != nullptr) {
			ly->ApplyExt(*pats);
		} else {
            throw std::runtime_error("ERROR: Input to layer \"" + ly->Name + "\" not found in environment.");
        }
	}
}

void leabra::Sim::NewRun() {
    Ctx->Reset();
    Net->InitWeights();
}

void leabra::Sim::ApplyParams() {
    Net->ApplyParams(*Params, true);
}

void leabra::Sim::Run(int numEpochs, bool train) {
    for (auto &[layerName, vector]: EpochSSE){
        vector.reserve(vector.size()+numEpochs);
    }

    for (int epochIndex = 0; epochIndex < numEpochs; epochIndex++) {
        StepEpoch(train);
    }
}

// Calls the SSE function on each TargetLayer type at the end of each trial
void leabra::Sim::RecordSSE() {
    for (auto &[layerName, vector]: TrialSSE){
        Layer *layer = (Layer*) Net->LayerNameMap[layerName];
        vector.push_back(layer->SSE());
    }
}

tensor::Tensor<float> *leabra::TabulatedEnv::GetLayerInput(std::string layerName) {
    // pattable::Event &currentEvent = table->events[table->eventNames[permutation[table->EventIndex]]];
    std::string eventName = table->eventNames[permutation[EventIndex]];
    return table->GetPattern(eventName, layerName);
}

bool leabra::TabulatedEnv::EndEpoch() {
    if (EventIndex >= numEvents) {
        Init();
        return true;
    } else {
        return false;
    }
}

int leabra::TabulatedEnv::NumTrials() {
    return numEvents;
}

leabra::Sim::Sim(Network *net, params::Sets *params, Environment *env):
    Net(net), Params(params), Env(env){
    Ctx = new leabra::Context();
}

// TrainTrial runs one trial of training using Env
void leabra::Sim::TrainTrial() {
	StepTrial(true); // run with training on
}

void leabra::Sim::StepTrial(bool train) {
    ApplyInputs();
	AlphaCyc(train);
    Env->Step(); // increments the environment state
    RecordSSE();
}

void leabra::Sim::StepEpoch(bool train) {
    while (!Env->EndEpoch()){
        StepTrial(train);
    }

    // TODO: find a way to compute this without interrupting sim execution
    for (auto &[layerName, sseVector]: TrialSSE) {
        float sum = 0;

        for (float &sse: sseVector) {
            sum += sse;
        }

        float meanSSE = sum / sseVector.size();
        EpochSSE[layerName].push_back(meanSSE);

        sseVector.clear(); // reset without clearing capacity
    }
}

leabra::TabulatedEnv::TabulatedEnv():permutation() {
    // Uninitialized Table...
}

leabra::TabulatedEnv::TabulatedEnv(std::string fileName) {
    table = new pattable::Table(fileName);
    numEvents = table->eventNames.size();
    permutation = rands::Perm(numEvents); // FIXME: produces multiple zeros
}

// For table environments Init starts the counter over
void leabra::TabulatedEnv::Init() {
    EventIndex = 0;
    if (shuffle) {
        permutation = rands::Perm(numEvents);
    }
}

void leabra::TabulatedEnv::Step() {
    EventIndex++;
    // if (EventIndex > numEvents) {
    //     Init();
    // }
}


// Initializes the table from the file name given
void leabra::TabulatedEnv::TableFromFile(std::string fileName) {
    table = new pattable::Table(fileName);
    numEvents = table->eventNames.size();
    permutation = rands::Perm(numEvents);
}
