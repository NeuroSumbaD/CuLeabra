#include "network.hpp"
#include "layer.hpp"

leabra::Network::Network(std::string name, int wtBalInterval):
	emer::Network(name), WtBalInterval(wtBalInterval) {
	NThreads = 1;WtBalCtr = 0;
}

int leabra::Network::NumLayers() {
	return Layers.size();
}

emer::Layer *leabra::Network::EmerLayer(int idx) {

    return (emer::Layer *)Layers[idx];
}

int leabra::Network::MaxParallelData(){return 1;}

int leabra::Network::NParallelData(){return 1;}

void leabra::Network::Defaults() {
    WtBalInterval=20;
    WtBalCtr=0;
    int index = 0;
    for (Layer *ly: Layers){
        ly->Defaults();
        ly->Index = index++;
    }
}

// UpdateParams updates all the derived parameters if any have changed, for all layers
// and pathways
void leabra::Network::UpdateParams() {
	for (Layer *ly: Layers) {
		ly->UpdateParams();
	}
}

// AddLayerInit is implementation routine that takes a given layer and
// adds it to the network, and initializes and configures it properly.
void leabra::Network::AddLayerInit(Layer *ly, std::string name, std::vector<int> shape, LayerTypes typ) {
	// emer::InitLayer(ly, name);
	*ly = leabra::Layer(name);
	ly->SetShape(shape);
	ly->Type = typ;
	Layers.push_back(ly);
	UpdateLayerMaps();
}

// AddLayer adds a new layer with given name and shape to the network.
// 2D and 4D layer shapes are generally preferred but not essential -- see
// AddLayer2D and 4D for convenience methods for those.  4D layers enable
// pool (unit-group) level inhibition in Leabra networks, for example.
// shape is in row-major format with outer-most dimensions first:
// e.g., 4D 3, 2, 4, 5 = 3 rows (Y) of 2 cols (X) of pools, with each unit
// group having 4 rows (Y) of 5 (X) units.
leabra::Layer* leabra::Network::AddLayer(std::string name, std::vector<int> shape, LayerTypes typ) {
	Layer *ly = nullptr;
	AddLayerInit(ly, name, shape, typ);
	return ly;
}

// AddLayer2D adds a new layer with given name and 2D shape to the network.
// 2D and 4D layer shapes are generally preferred but not essential.
leabra::Layer *leabra::Network::AddLayer2D(std::string name, int shapeY, int shapeX, LayerTypes typ) {
	return AddLayer(name, {shapeY, shapeX}, typ);
}

// AddLayer4D adds a new layer with given name and 4D shape to the network.
// 4D layers enable pool (unit-group) level inhibition in Leabra networks, for example.
// shape is in row-major format with outer-most dimensions first:
// e.g., 4D 3, 2, 4, 5 = 3 rows (Y) of 2 cols (X) of pools, with each pool
// having 4 rows (Y) of 5 (X) neurons.
leabra::Layer *leabra::Network::AddLayer4D(std::string name, int nPoolsY, int nPoolsX, int nNeurY, int nNeurX, LayerTypes typ) {
    return AddLayer(name, {nPoolsY, nPoolsX, nNeurY, nNeurX}, typ);
}

std::tuple<leabra::Layer *, leabra::Layer *, leabra::Path *> leabra::Network::ConnectLayerNames(std::string send, std::string recv, paths::Pattern *pat, PathTypes typ) {
	leabra::Layer *rlay, *slay;
	leabra::Path *pt;

	rlay = (leabra::Layer*)LayerByName(recv);
	if (rlay == nullptr) {
		throw std::invalid_argument("Recv layer name (" + recv + ") not found.");
	}
	slay = (leabra::Layer*)LayerByName(send);
	if (slay == nullptr) {
		throw std::invalid_argument("Send layer name (" + send + ") not found.");
	}
	pt = ConnectLayers(slay, rlay, pat, typ);

    return std::tuple<leabra::Layer *, leabra::Layer *, leabra::Path *>(rlay, slay, pt);
}

// ConnectLayers establishes a pathway between two layers,
// adding to the recv and send pathway lists on each side of the connection.
// Does not yet actually connect the units within the layers -- that
// requires Build.
leabra::Path *leabra::Network::ConnectLayers(Layer *send, Layer *recv, paths::Pattern *pat, PathTypes typ) {
	leabra::Path *pt = new leabra::Path();
	pt->Connect(send, recv, pat, typ);
	recv->RecvPaths.push_back(*pt);
	send->SendPaths.push_back(*pt);
	return pt;
}

// BidirConnectLayerNames establishes bidirectional pathways between two layers,
// referenced by name, with low = the lower layer that sends a Forward pathway
// to the high layer, and receives a Back pathway in the opposite direction.
// Returns error if not successful.
// Does not yet actually connect the units within the layers -- that requires Build.
std::tuple<leabra::Layer *, leabra::Layer *, leabra::Path *, leabra::Path *> leabra::Network::BidirConnectLayerNames(std::string low, std::string high, paths::Pattern *pat) {
	leabra::Layer *lowlay, *highlay;
	leabra::Path *fwdpj, *backpj;

	lowlay = (leabra::Layer*) LayerByName(low);
	if (lowlay == nullptr) {
		throw std::invalid_argument("Low layer name (" + low + ") not found.");
	}
	highlay = (leabra::Layer*) LayerByName(high);
	if (highlay == nullptr) {
		throw std::invalid_argument("High layer name (" + high + ") not found.");
	}
	fwdpj = ConnectLayers(lowlay, highlay, pat, leabra::ForwardPath);
	backpj = ConnectLayers(highlay, lowlay, pat, leabra::BackPath);

    return std::tuple<leabra::Layer *, leabra::Layer *, leabra::Path *, leabra::Path *>(lowlay, highlay, fwdpj, backpj);
}

// BidirConnectLayers establishes bidirectional pathways between two layers,
// with low = lower layer that sends a Forward pathway to the high layer,
// and receives a Back pathway in the opposite direction.
// Does not yet actually connect the units within the layers -- that
// requires Build.
std::tuple<leabra::Path *, leabra::Path *> leabra::Network::BidirConnectLayers(Layer *low, Layer *high, paths::Pattern *pat) {
	leabra::Path *fwdpj, *backpj;
	fwdpj = ConnectLayers(low, high, pat, ForwardPath);
	backpj = ConnectLayers(high, low, pat, BackPath);
    return std::tuple<leabra::Path *, leabra::Path *>(fwdpj, backpj);
}

// LateralConnectLayer establishes a self-pathway within given layer.
// Does not yet actually connect the units within the layers -- that
// requires Build.
leabra::Path *leabra::Network::LateralConnectLayer(Layer *lay, paths::Pattern *pat) {
	leabra::Path *pt = nullptr;
    return LateralConnectLayerPath(lay, pat, pt);
}

// LateralConnectLayerPath makes lateral self-pathway using given pathway.
// Does not yet actually connect the units within the layers -- that
// requires Build.
leabra::Path *leabra::Network::LateralConnectLayerPath(Layer *lay, paths::Pattern *pat, Path *pt) {
	pt->Connect(lay, lay, pat, LateralPath);
	// TODO: Check if there are consequences to copying pt
	lay->RecvPaths.push_back(*pt);	
	lay->SendPaths.push_back(*pt);
	return pt;
}

// Build constructs the layer and pathway state based on the layer shapes
// and patterns of interconnectivity
void leabra::Network::Build() {
	UpdateLayerMaps();
	std::vector<std::string> errs = std::vector<std::string>();
	for (uint li = 0; li < Layers.size(); li ++) {
		Layer &ly = *Layers[li];
		ly.Index = li;
		if (ly.Off) {
			continue;
		}
		ly.Build();
	}
	LayoutLayers();
}

// AlphaCycInit handles all initialization at start of new input pattern.
// Should already have presented the external input to the network at this point.
// If updtActAvg is true, this includes updating the running-average
// activations for each layer / pool, and the AvgL running average used
// in BCM Hebbian learning.
// The input scaling is updated  based on the layer-level running average acts,
// and this can then change the behavior of the network,
// so if you want 100% repeatable testing results, set this to false to
// keep the existing scaling factors (e.g., can pass a train bool to
// only update during training).
// This flag also affects the AvgL learning threshold.
void leabra::Network::AlphaCycInit(bool updtActAvg) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->AlphaCycInit(updtActAvg);
	}
}

// Cycle runs one cycle of activation updating:
// * Sends Ge increments from sending to receiving layers
// * Average and Max Ge stats
// * Inhibition based on Ge stats and Act Stats (computed at end of Cycle)
// * Activation from Ge, Gi, and Gl
// * Average and Max Act stats
// This basic version doesn't use the time info, but more specialized types do, and we
// want to keep a consistent API for end-user code.
void leabra::Network::Cycle(Context *ctx) {
    SendGDelta(ctx); // also does integ
	AvgMaxGe(ctx);
	InhibFromGeAct(ctx);
	ActFromG(ctx);
	AvgMaxAct(ctx);
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->CyclePost(ctx);
	}
}

// SendGeDelta sends change in activation since last sent, if above thresholds
// and integrates sent deltas into GeRaw and time-integrated Ge values
void leabra::Network::SendGDelta(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->SendGDelta(ctx);
	}
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->GFromInc(ctx);
	}
}

// AvgMaxGe computes the average and max Ge stats, used in inhibition
void leabra::Network::AvgMaxGe(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->AvgMaxGe(ctx);
	}
}

// InhibiFromGeAct computes inhibition Gi from Ge and Act stats within relevant Pools
void leabra::Network::InhibFromGeAct(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InhibFromGeAct(ctx);
	}
}

// ActFromG computes rate-code activation from Ge, Gi, Gl conductances
void leabra::Network::ActFromG(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->ActFromG(ctx);
	}
}

// QuarterFinal does updating after end of a quarter, for first 2
void leabra::Network::AvgMaxAct(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->AvgMaxAct(ctx);
	}
}

// QuarterFinal does updating after end of a quarter, for first 2
void leabra::Network::QuarterFinal(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->QuarterFinal(ctx);
	}
}

// MinusPhase is called at the end of the minus phase (quarter 3), to record state.
void leabra::Network::MinusPhase(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->MinusPhase(ctx);
	}
}

// PlusPhase is called at the end of the plus phase (quarter 4), to record state.
void leabra::Network::PlusPhase(Context *ctx) {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->PlusPhase(ctx);
	}
}

// DWt computes the weight change (learning) based on current
// running-average activation values
void leabra::Network::Dwt() {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->DWt();
	}
}

// WtFromDWt updates the weights from delta-weight changes.
// Also calls WtBalFromWt every WtBalInterval times
void leabra::Network::WtFromDwt() {
    for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->WtFromDWt();
	}

	WtBalCtr++;
	if (WtBalCtr >= WtBalInterval) {
		WtBalCtr = 0;
        for (Layer *ly: Layers) {
            if (ly->Off) {
                continue;
            }
            ly->WtBalFromWt();
        }
	}
}

// LrateMult sets the new Lrate parameter for Paths to LrateInit * mult.
// Useful for implementing learning rate schedules.
void leabra::Network::LrateMult(float mult) {
    for (Layer *ly: Layers) {
		// if (ly->Off) {
		// 	continue;
		// }
		ly->LrateMult(mult);
	}
}

// InitWeights initializes synaptic weights and all other
// associated long-term state variables including running-average
// state values (e.g., layer running average activations etc).
void leabra::Network::InitWeights() {
    WtBalCtr = 0;
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InitWeights();
	}
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InitWtSym();
	}
}

// InitTopoScales initializes synapse-specific scale parameters from
// path types that support them, with flags set to support it,
// includes: paths.PoolTile paths.Circle.
// call before InitWeights if using Topo wts.
void leabra::Network::InitTopoScales() {
	auto shp = std::vector<int>();
    auto scales = tensor::Tensor<float>(shp);
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		auto &rpjn = ly->RecvPaths;
		for (Path &pt: rpjn) {
			if (pt.Off) {
				continue;
			}
			paths::Pattern *pat = pt.Pattern;
			if (pat->type == "PoolTile"){
				auto ptn = (paths::PoolTile*) pat;
				if (!ptn->HasTopoWeights()) {
					continue;
				}
				Layer &slay = *pt.Send;
				ptn->TopoWeights(slay.Shape, ly->Shape, scales);
				pt.SetScalesRPool(scales);
			} else if (pat->type == "Circle"){
				auto ptn = (paths::Circle*) pat;
				if (!ptn->TopoWeights) {
					continue;
				}
				pt.SetScalesFunc(ptn->GaussWts);
			}
		}
	}
}

// DecayState decays activation state by given proportion
// e.g., 1 = decay completely, and 0 = decay not at all
// This is called automatically in AlphaCycInit, but is avail
// here for ad-hoc decay cases.
void leabra::Network::DecayState(float decay) {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->DecayState(decay);
	}
}

// InitActs fully initializes activation state -- not automatically called
void leabra::Network::InitActs() {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InitActs();
	}
}

// InitExt initializes external input state.
// call prior to applying external inputs to layers.
void leabra::Network::InitExt() {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InitExt();
	}
}

// UpdateExtFlags updates the neuron flags for external input
// based on current layer Type field.
// call this if the Type has changed since the last
// ApplyExt* method call.
void leabra::Network::UpdateExtFlags() {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->UpdateExtFlags();
	}
}

// InitGinc initializes the Ge excitatory and Gi inhibitory
// conductance accumulation states including ActSent and G*Raw values.
// called at start of trial always (at layer level), and can be
// called optionally when delta-based Ge computation needs
// to be updated (e.g., weights might have changed strength).
void leabra::Network::InitGInc() {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->InitGInc();
	}
}

// GScaleFromAvgAct computes the scaling factor for synaptic input conductances G,
// based on sending layer average activation.
// This attempts to automatically adjust for overall differences in raw activity
// coming into the units to achieve a general target of around .5 to 1
// for the integrated Ge value.
// This is automatically done during AlphaCycInit, but if scaling parameters are
// changed at any point thereafter during AlphaCyc, this must be called.
void leabra::Network::GScaleFromAvgAct() {
	for (Layer *ly: Layers) {
		if (ly->Off) {
			continue;
		}
		ly->GScaleFromAvgAct();
	}
}

// LayersSetOff sets the Off flag for all layers to given setting
void leabra::Network::LayersSetOff(bool off) {
	for (Layer *ly: Layers) {
		ly->Off = off;
	}
}

// UnLesionNeurons unlesions neurons in all layers in the network.
// Provides a clean starting point for subsequent lesion experiments.
void leabra::Network::UnLesionNeurons() {
	for (Layer *ly: Layers) {
		ly->UnLesionNeurons();
	}
}
