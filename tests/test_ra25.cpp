#include "leabra.hpp"
#include "network.hpp"
#include "layer.hpp"
#include "path.hpp"
#include "params.hpp"

// TODO: Find a way to clean up this syntax
// TODO: Find a way to create and read JSON param files
params::Sets ParamSets = 
    {
        {
            "Base",
            {
                {
                    Sel: "Path",
                    Desc: "norm and momentum on works better, but wt bal is not better for smaller nets",
                    ParamsSet: {
                        {"Path.Learn.Norm.On",     "true"},
                        {"Path.Learn.Momentum.On", "true"},
                        {"Path.Learn.WtBal.On",    "true"}, // no diff really
                        // "Path.Learn.WtBal.Targs": "true", // no diff here
                    }
                },
                {
                    Sel: "Layer",
                    Desc: "using default 1.8 inhib for all of network -- can explore",
                    ParamsSet: {
                        {"Layer.Inhib.Layer.Gi", "1.8"},
                        {"Layer.Act.Init.Decay", "0.0"},
                        {"Layer.Act.Gbar.L",     "0.1"}, // set explictly, new default, a bit better vs 0.2
                    }
                },
                {
                    Sel: ".BackPath",
                    Desc: "top-down back-pathways MUST have lower relative weight scale, otherwise network hallucinates",
                    ParamsSet: {
                        {"Path.WtScale.Rel", "0.2"},
                    }
                },
                {
                    Sel: "#Output",
                    Desc: "output definitely needs lower inhib -- true for smaller layers in general",
                    ParamsSet: {
                        {"Layer.Inhib.Layer.Gi", "1.4"},
                    }
                },
            },
        },
        {
            "DefaultInhib", 
            {
                {
                    Sel: "#Output",
                    Desc: "go back to default",
                    ParamsSet: {
                        {"Layer.Inhib.Layer.Gi", "1.8"},
                    }
                },
            }
        },
        {
            "NoMomentum",
            {
                {
                    Sel: "Path",
                    Desc: "no norm or momentum",
                    ParamsSet: {
                        {"Path.Learn.Norm.On",     "false"},
                        {"Path.Learn.Momentum.On", "false"},
                    }
                },
            }
        },
        {
            "WtBalOn",
            {
                {
                    Sel: "Path",
                    Desc: "weight bal on",
                    ParamsSet: {
                        {"Path.Learn.WtBal.On", "true"},
                    }
                },
            }
        },
};

void ConfigNet(leabra::Network *net, int Hidden1SizeX = 7, int Hidden1SizeY = 7, int Hidden2SizeX = 7, int Hidden2SizeY = 7) {
    // Seed automatically created from constructor
    // net->SetRandSeed(ss.RandSeeds[0]) // init new separate random seed, using run = 0

	leabra::Layer *inp = net->AddLayer2D("Input", 5, 5, leabra::InputLayer);
    // Doc string not yet implemented (low priority)
	// inp->Doc = "Input represents sensory input, coming into the cortex via tha thalamus"; 
	leabra::Layer *hid1 = net->AddLayer2D("Hidden1", Hidden1SizeY, Hidden1SizeX, leabra::SuperLayer);
	// hid1.Doc = "First hidden layer performs initial internal processing of sensory inputs, transforming in preparation for producing appropriate responses"
	leabra::Layer *hid2 = net->AddLayer2D("Hidden2", Hidden2SizeY, Hidden2SizeX, leabra::SuperLayer);
	// hid2.Doc = "Another 'deep' layer of internal processing to prepare directly for Output response"
	leabra::Layer *out = net->AddLayer2D("Output", 5, 5, leabra::TargetLayer);
	// out.Doc = "Output represents motor output response, via deep layer 5 neurons projecting supcortically, in motor cortex"

    // Positioning not yet implemented (will probably default to left-to-right placement)
	// use this to position layers relative to each other
	// hid2.PlaceRightOf(hid1, 2)

	// note: see emergent/path module for all the options on how to connect
	// NewFull returns a new paths.Full connectivity pattern
	paths::Pattern *full = new paths::Full;

	net->ConnectLayers(inp, hid1, full, leabra::ForwardPath);
	net->BidirConnectLayers(hid1, hid2, full);
	net->BidirConnectLayers(hid2, out, full);

	// net->LateralConnectLayerPath(hid1, full, &leabra.HebbPath{}).SetType(InhibPath)

	// note: if you wanted to change a layer type from e.g., Target to Compare, do this:
	// out.SetType(emer.Compare)
	// that would mean that the output layer doesn't reflect target values in plus phase
	// and thus removes error-driven learning -- but stats are still computed.

	net->Build();
	net->Defaults();
    net->ApplyParams(ParamSets, true);
	net->InitWeights();
}

int main(){
    leabra::Network *net = new leabra::Network("RA25");

    return 0;
}