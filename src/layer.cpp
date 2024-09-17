#include "layer.hpp"

leabra::Layer::Layer(std::string name, int index, Network *net): 
	emer::Layer(name), Index(index), Net(net), RecvPaths(), SendPaths(), Act(), Inhib(), Learn(), Neurons(), Pools(), CosDiff() {
	Inhib.Layer.On = true;
}

void leabra::Layer::Defaults() {
    Act.Defaults();
    Inhib.Defaults();
    Learn.Defaults();
    Inhib.Layer.On = true;
    for (Path &pt: RecvPaths) {
        pt.Defaults();
    }
}

void leabra::Layer::UpdateParams() {
    Act.Update();
    Inhib.Update();
    Learn.Update();
    Inhib.Layer.On = true;
    for (Path &pt: RecvPaths) {
        pt.UpdateParams();
    }
}

leabra::Pool *leabra::Layer::GetPool(int idx) {
	return &(Pools[idx]);
}

// BuildSubPools initializes neuron start / end indexes for sub-pools
void leabra::Layer::BuildSubPools() {
	if (!Is4D()) {
		return;
	}
	std::vector<int> &sh = Shape.Sizes;
	int spy = sh[0];
	int spx = sh[1];
	int pi = 1;
	for (int py = 0; py < spy; py++) {
		for (int px = 0; px < spx; px++) {
			int soff = Shape.Offset({py, px, 0, 0});
			int eoff = Shape.Offset({py, px, sh[2] - 1, sh[3] - 1}) + 1;
			Pool &pl = Pools[pi];
			pl.StIndex = soff;
			pl.EdIndex = eoff;
			for (int ni = pl.StIndex; ni < pl.EdIndex; ni++) {
				Neuron &nrn = Neurons[ni];
				nrn.SubPool = pi;
			}
			pi++;
		}
	}
}

// BuildPools builds the inhibitory pools structures -- nu = number of units in layer
void leabra::Layer::BuildPools(int nu) {
	int np = 1 + NumPools();
	Pools.reserve(np);
	Pool &lpl = Pools[0];
	lpl.StIndex = 0;
	lpl.EdIndex = nu;
	if (np > 1) {
		BuildSubPools();
	}
}

// BuildPaths builds the pathways, recv-side
void leabra::Layer::BuildPaths() {
	for (Path &pt: RecvPaths) {
		if (pt.Off) {
			continue;
		}
		pt.Build();
	}
}

// Build constructs the layer state, including calling Build on the pathways
void leabra::Layer::Build() {
	int nu = Shape.Len();
	if (nu == 0) {
		std::cerr << "Build Layer "<< Name <<": no units specified in Shape" << std::endl;
	}
	Neurons.reserve(nu);
	BuildPools(nu);
	
	BuildPaths();
}

// InitWeights initializes the weight values in the network,
// i.e., resetting learning Also calls InitActs.
void leabra::Layer::InitWeights() {
	UpdateParams();
	for (Path &pt: SendPaths) {
		if (pt.Off) {
			continue;
		}
        pt.InitWeights();
	}
    for (uint pi = 0; pi < Pools.size(); pi++) {
		leabra::Pool &pl = Pools[pi];
		pl.ActAvgs.ActMAvg = Inhib.ActAvg.Init;
		pl.ActAvgs.ActPAvg = Inhib.ActAvg.Init;
		pl.ActAvgs.ActPAvgEff = Inhib.ActAvg.EffInit();
	}
	InitActAvg();
	InitActs();
	CosDiff.Init();
}

/*-------TODO: CHECK IF INIT FUNCS ARE ACTUALLY USEFUL IN C++ -------*/
// If this is mostly initialization it may be unnecessary in C++, 
// but if it is used for resetting values to zero, then it will be necessary

// InitActAvg initializes the running-average activation
// values that drive learning.
void leabra::Layer::InitActAvg() {
    // for (int ni = 0; ni < Neurons.size(); ni++) {
    for (leabra::Neuron &nrn: Neurons) {
		// leabra::Neuron &nrn = Neurons[ni];
		Learn.InitActAvg(nrn);
	}
}


// InitActs fully initializes activation state.
// only called automatically during InitWeights.
void leabra::Layer::InitActs() {
    for (leabra::Neuron &nrn: Neurons) {
		// nrn := &ly.Neurons[ni]
		Act.InitActs(nrn);
	}
	for (leabra::Pool &pl: Pools) {
		// pl := &ly.Pools[pi]
		pl.Inhib.Init();
		pl.ActM.Init();
		pl.ActP.Init();
	}
}

// InitWeightsSym initializes the weight symmetry.
// higher layers copy weights from lower layers.
void leabra::Layer::InitWtSym() { //TODO: check if this is useful...
    for (leabra::Path &pt: SendPaths) {
        if (pt.Off) {
			continue;
		}
		if (!pt.WtInit.Sym) {
			continue;
		}
		// key ordering constraint on which way weights are copied
		if (pt.Recv->Index < pt.Send->Index) {
			continue;
		}
		// rpt= RecipToSendPath(pt);
		// if !has {
		// 	continue
		// }
		// if !(rpt.WtInit.Sym) {
		// 	continue
		// }
		// pt.InitWtSym(rpt)
    }
}

// InitExt initializes external input state -- called prior to apply ext
void leabra::Layer::InitExt() {
    for (leabra::Neuron &nrn: Neurons) {
        nrn.Ext = 0;
		nrn.Targ = 0;
		nrn.SetFlag(false, {NeurHasExt, NeurHasTarg, NeurHasCmpr});
    }
}

// ApplyExtFlags gets the flags that should cleared and set for updating neuron flags
// based on layer type, and whether input should be applied to Targ (else Ext)
std::tuple<std::vector<int>, std::vector<int>, bool> leabra::Layer::ApplyExtFlags() {
    std::vector<int> clear = {NeurHasExt, NeurHasTarg, NeurHasCmpr};
    bool toTarg = false;
    std::vector<int> set;
    if (Type == TargetLayer) {
		set = {NeurHasTarg};
		toTarg = true;
	} else if (Type == CompareLayer) {
		set = {NeurHasCmpr};
		toTarg = true;
	} else {
		set = {NeurHasExt};
	}
    return std::tuple<std::vector<int>, std::vector<int>, bool>(clear, set, toTarg);
}

// ApplyExt applies external input in the form of an tensor.Float32.  If
// dimensionality of tensor matches that of layer, and is 2D or 4D, then each dimension
// is iterated separately, so any mismatch preserves dimensional structure.
// Otherwise, the flat 1D view of the tensor is used.
// If the layer is a Target or Compare layer type, then it goes in Targ
// otherwise it goes in Ext
void leabra::Layer::ApplyExt(tensor::Tensor<float> ext) {
	if (ext.NumDims() == 2 && Shape.NumDims() == 4) { // special case
		ApplyExt2Dto4D(ext);
	} else if (ext.NumDims() != Shape.NumDims() || !(ext.NumDims() == 2 || ext.NumDims() == 4)) {
		ApplyExt1DTsr(ext.Values);
	} else if (ext.NumDims() == 2){
		ApplyExt2D(ext);
	} else if (ext.NumDims() == 4) {
		ApplyExt4D(ext);
	}
}

// ApplyExt2D applies 2D tensor external input
void leabra::Layer::ApplyExt2D(tensor::Tensor<float> ext) {
	// clear, set, toTarg := ApplyExtFlags();
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);
	bool toTarg = std::get<2>(flagsTuple);

	float ymx = std::min(ext.Shp.Sizes[0], Shape.Sizes[0]);
	float xmx = std::min(ext.Shp.Sizes[1], Shape.Sizes[1]);
	for (int y = 0; y < ymx; y++) {
		for (int x = 0; x < xmx; x++) {
			std::vector<int> idx = {y, x};
			int i = Shape.Offset(idx);
			float vl = ext.Values[i];
			Neuron &nrn = Neurons[i];
			if (nrn.IsOff()) {
				continue;
			}
			if (toTarg) {
				nrn.Targ = vl;
			} else {
				nrn.Ext = vl;
			}
			nrn.SetFlag(false, clear);
			nrn.SetFlag(true, set);
		}
	}
}

// ApplyExt2Dto4D applies 2D tensor external input to a 4D layer
void leabra::Layer::ApplyExt2Dto4D(tensor::Tensor<float> ext) {
	// clear, set, toTarg := ly.ApplyExtFlags()
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);
	bool toTarg = std::get<2>(flagsTuple);

	// lNy, lNx, _, _ := tensor:Projection2DShape(&ly.Shape, false)
	auto projectionTuple = tensor::Projection2DShape(Shape, false);
	int lNy = std::get<0>(projectionTuple);
	int lNx = std::get<1>(projectionTuple);

	float ymx = std::min(ext.Shp.Sizes[0], lNy);
	float xmx = std::min(ext.Shp.Sizes[1], lNx);
	for (int y = 0; y < ymx; y++) {
		for (int x = 0; x < xmx; x++) {
			std::vector<int> idx = {y, x};
			int i = Shape.Offset(idx);
			float vl = ext.Values[i];
			int ui = tensor::Projection2DIndex(Shape, false, y, x);
			Neuron &nrn = Neurons[ui];
			if (nrn.IsOff()) {
				continue;
			}
			if (toTarg) {
				nrn.Targ = vl;
			} else {
				nrn.Ext = vl;
			}
			nrn.SetFlag(false, clear);
			nrn.SetFlag(true, set);
		}
	}
}

// ApplyExt4D applies 4D tensor external input
void leabra::Layer::ApplyExt4D(tensor::Tensor<float> ext) {
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);
	bool toTarg = std::get<2>(flagsTuple);

	int ypmx = std::min(ext.DimSize(0), Shape.DimSize(0));
	int xpmx = std::min(ext.DimSize(1), Shape.DimSize(1));
	int ynmx = std::min(ext.DimSize(2), Shape.DimSize(2));
	int xnmx = std::min(ext.DimSize(3), Shape.DimSize(3));
	for (int yp = 0; yp < ypmx; yp++) {
		for (int xp = 0; xp < xpmx; xp++) {
			for (int yn = 0; yn < ynmx; yn++) {
				for (int xn = 0; xn < xnmx; xn++) {
					std::vector<int> idx = {yp, xp, yn, xn};
					int i = Shape.Offset(idx);
					float vl = ext.Values[i];
					Neuron &nrn = Neurons[i];
					if (nrn.IsOff()) {
						continue;
					}
					if (toTarg) {
						nrn.Targ = vl;
					} else {
						nrn.Ext = vl;
					}
					nrn.SetFlag(false, clear);
					nrn.SetFlag(true, set);
				}
			}
		}
	}
}

// ApplyExt1DTsr applies external input using 1D flat interface into tensor.
// If the layer is a Target or Compare layer type, then it goes in Targ
// otherwise it goes in Ext
void leabra::Layer::ApplyExt1DTsr(std::vector<float> ext) {
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);
	bool toTarg = std::get<2>(flagsTuple);

	int mx = std::min(ext.size(), Neurons.size());
	for (int i = 0; i < mx; i++) {
		Neuron &nrn = Neurons[i];
		if (nrn.IsOff()) {
			continue;
		}
		float vl = ext[i];
		if (toTarg) {
			nrn.Targ = vl;
		} else {
			nrn.Ext = vl;
		}
		nrn.SetFlag(false, clear);
		nrn.SetFlag(true, set);
	}
}

// ApplyExt1D applies external input in the form of a flat 1-dimensional slice of floats
// If the layer is a Target or Compare layer type, then it goes in Targ
// otherwise it goes in Ext
void leabra::Layer::ApplyExt1D(std::vector<float> ext) {
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);
	bool toTarg = std::get<2>(flagsTuple);

	int mx = std::min(ext.size(), Neurons.size());
	for (int i = 0; i < mx; i++) {
		Neuron &nrn = Neurons[i];
		if (nrn.IsOff()) {
			continue;
		}
		float vl = ext[i];
		if (toTarg) {
			nrn.Targ = vl;
		} else {
			nrn.Ext = vl;
		}
		nrn.SetFlag(false, clear);
		nrn.SetFlag(true, set);
	}
}

// UpdateExtFlags updates the neuron flags for external input based on current
// layer Type field -- call this if the Type has changed since the last
// ApplyExt* method call.
void leabra::Layer::UpdateExtFlags() {
	std::tuple<std::vector<int>, std::vector<int>, bool> flagsTuple = ApplyExtFlags();
	std::vector<int> clear = std::get<0>(flagsTuple);
	std::vector<int> set = std::get<1>(flagsTuple);

	for (Neuron &nrn: Neurons){
		if (nrn.IsOff()){
			continue;
		}
		nrn.SetFlag(false, clear);
		nrn.SetFlag(true, set);
	}
}

// ActAvgFromAct updates the running average ActMAvg, ActPAvg, and ActPAvgEff
// values from the current pool-level averages.
// The ActPAvgEff value is used for updating the conductance scaling parameters,
// if these are not set to Fixed, so calling this will change the scaling of
// pathways in the network!
void leabra::Layer::ActAvgFromAct() {
	for (Pool &pl: Pools){
		Inhib.ActAvg.AvgFromAct(pl.ActAvgs.ActMAvg, pl.ActM.Avg);
		Inhib.ActAvg.AvgFromAct(pl.ActAvgs.ActPAvg, pl.ActP.Avg);
		Inhib.ActAvg.EffFromAvg(pl.ActAvgs.ActPAvgEff, pl.ActAvgs.ActPAvg);
	}
}

// ActQ0FromActP updates the neuron ActQ0 value from prior ActP value
void leabra::Layer::ActQ0FromActP() {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		nrn.ActQ0 = nrn.ActP;
	}
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
// only update during training).  This flag also affects the AvgL learning
// threshold
void leabra::Layer::AlphaCycInit(bool updtActAvg) {
	ActQ0FromActP();
	if (updtActAvg) {
		AvgLFromAvgM();
		ActAvgFromAct();
	}
	GScaleFromAvgAct(); // need to do this always, in case hasn't been done at all yet
	if (Act.Noise.Fixed && Act.Noise.DistType != rands::Mean) {
		GenNoise();
	}
	DecayState(Act.Init.Decay);
	InitGInc();
	if (Act.Clamp.Hard && Type == InputLayer) {
		HardClamp();
	}
}

// AvgLFromAvgM updates AvgL long-term running average activation that drives BCM Hebbian learning
void leabra::Layer::AvgLFromAvgM() {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Learn.AvgLFromAvgM(nrn);
		if (Learn.AvgL.ErrMod) {
			nrn.AvgLLrn *= CosDiff.ModAvgLLrn;
		}
	}
}

// GScaleFromAvgAct computes the scaling factor for synaptic input conductances G,
// based on sending layer average activation.
// This attempts to automatically adjust for overall differences in raw activity
// coming into the units to achieve a general target of around .5 to 1
// for the integrated Ge value.
void leabra::Layer::GScaleFromAvgAct() {
	float totGeRel = 0;
	float totGiRel = 0;
	for (Path &pt : RecvPaths) {
		if (pt.Off) {
			continue;
		}
		Layer &slay = *pt.Send;
		Pool &slpl = slay.Pools[0];
		float savg = slpl.ActAvgs.ActPAvgEff;
		int snu = slay.Neurons.size();
		int ncon = pt.RConNAvgMax.Avg;
		pt.GScale = pt.WtScale.FullScale(savg, float(snu), ncon);

		if (pt.Type == InhibPath) {
			totGiRel += pt.WtScale.Rel;
		} else {
			totGeRel += pt.WtScale.Rel;
		}
	}

	for (Path &pt : RecvPaths) {
		if (pt.Off) {
			continue;
		}
		if (pt.Type == InhibPath) {
			if (totGiRel > 0) {
				pt.GScale /= totGiRel;
			}
		} else {
			if (totGeRel > 0) {
				pt.GScale /= totGeRel;
			}
		}
	}
}

// GenNoise generates random noise for all neurons
void leabra::Layer::GenNoise() {
	for (Neuron &nrn: Neurons) {
		nrn.Noise = Act.Noise.Gen();
	}
}

// DecayState decays activation state by given proportion (default is on ly.Act.Init.Decay).
// This does *not* call InitGInc -- must call that separately at start of AlphaCyc
void leabra::Layer::DecayState(float decay) {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Act.DecayState(nrn, decay);
	}
	for (Pool &pl: Pools) { // decaying average act is essential for inhib
		pl.Inhib.Decay(decay);
	}
}

// DecayStatePool decays activation state by given proportion
// in given sub-pool index (0 based).
void leabra::Layer::DecayStatePool(int pool, float decay) {
	int pi = pool + 1; // 1 based TODO: CHECK IF THIS IS A GO SPECIFIC
	Pool &pl = Pools[pi];
	for (int ni = pl.StIndex; ni < pl.EdIndex; ni++) {
		Neuron &nrn = Neurons[ni];
		if (nrn.IsOff()) {
			continue;
		}
		Act.DecayState(nrn, decay);
	}
	pl.Inhib.Decay(decay);
}

// HardClamp hard-clamps the activations in the layer.
// called during AlphaCycInit for hard-clamped Input layers.
void leabra::Layer::HardClamp() {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Act.HardClamp(nrn);
	}
}

// InitGinc initializes the Ge excitatory and Gi inhibitory conductance accumulation states
// including ActSent and G*Raw values.
// called at start of trial always, and can be called optionally
// when delta-based Ge computation needs to be updated (e.g., weights
// might have changed strength)
void leabra::Layer::InitGInc() {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Act.InitGInc(nrn);
	}
	for (Path &pt: RecvPaths) {
		if (pt.Off) {
			continue;
		}
		pt.InitGInc();
	}
}

// SendGDelta sends change in activation since last sent, to increment recv
// synaptic conductances G, if above thresholds
void leabra::Layer::SendGDelta(Context *ctx) {
	for (uint ni = 0; ni < Neurons.size(); ni++) {
		Neuron &nrn = Neurons[ni];
		if (nrn.IsOff()) {
			continue;
		}
		if (nrn.Act > Act.OptThresh.Send) {
			float delta = nrn.Act - nrn.ActSent;
			if (std::abs(delta) > Act.OptThresh.Delta) {
				for (Path &sp: SendPaths) {
					if (sp.Off) {
						continue;
					}
					sp.SendGDelta(ni, delta);
				}
				nrn.ActSent = nrn.Act;
			}
		} else if (nrn.ActSent > Act.OptThresh.Send) {
			float delta = -nrn.ActSent; // un-send the last above-threshold activation to get back to 0
			for (Path &sp: SendPaths) {
				if (sp.Off) {
					continue;
				}
				sp.SendGDelta(ni, delta);
			}
			nrn.ActSent = 0;
		}
	}
}

// GFromInc integrates new synaptic conductances from increments sent during last SendGDelta.
void leabra::Layer::GFromInc(Context *ctx) {
	RecvGInc(ctx);
	GFromIncNeur(ctx);
}

// RecvGInc calls RecvGInc on receiving pathways to collect Neuron-level G*Inc values.
// This is called by GFromInc overall method, but separated out for cases that need to
// do something different.
void leabra::Layer::RecvGInc(Context *ctx) {
	for (Path &pt: RecvPaths) {
		if (pt.Off) {
			continue;
		}
		pt.RecvGInc();
	}
}

// GFromIncNeur is the neuron-level code for GFromInc that integrates overall Ge, Gi values
// from their G*Raw accumulators.
void leabra::Layer::GFromIncNeur(Context *ctx) {
	for (uint ni = 0; ni < Neurons.size(); ni++) {
		Neuron &nrn = Neurons[ni];
		if (nrn.IsOff()) {
			continue;
		}
		// note: each step broken out here so other variants can add extra terms to Raw
		Act.GeFromRaw(nrn, nrn.GeRaw);
		Act.GiFromRaw(nrn, nrn.GiRaw);
	}
}

// AvgMaxGe computes the average and max Ge stats, used in inhibition
void leabra::Layer::AvgMaxGe(Context *ctx) {
	for (Pool &pl: Pools) {
		pl.Inhib.Ge.Init();
		for (int ni = pl.StIndex; ni < pl.EdIndex; ni++) {
			Neuron &nrn = Neurons[ni];
			if (nrn.IsOff()) {
				continue;
			}
			pl.Inhib.Ge.UpdateValue(nrn.Ge, ni);
		}
		pl.Inhib.Ge.CalcAvg();
	}
}

// InhibFromGeAct computes inhibition Gi from Ge and Act averages within relevant Pools
void leabra::Layer::InhibFromGeAct(Context *ctx) {
	Pool &lpl = Pools[0];
	Inhib.Layer.Inhib(&lpl.Inhib);
	PoolInhibFromGeAct(ctx);
	InhibFromPool(ctx);
}

// PoolInhibFromGeAct computes inhibition Gi from Ge and Act averages within relevant Pools
void leabra::Layer::PoolInhibFromGeAct(Context *ctx) {
	int np = Pools.size();
	if (np == 1) {
		return;
	}
	Pool &lpl = Pools[0];
	bool lyInhib = Inhib.Layer.On;
	for (int pi = 1; pi < np; pi++) {
		Pool &pl = Pools[pi];
		Inhib.Pool.Inhib(&pl.Inhib);
		if (lyInhib) {
			pl.Inhib.LayGi = lpl.Inhib.Gi;
			pl.Inhib.Gi = std::max(pl.Inhib.Gi, lpl.Inhib.Gi); // pool is max of layer
		} else {
			lpl.Inhib.Gi = std::max(pl.Inhib.Gi, lpl.Inhib.Gi); // update layer from pool
		}
	}
	if (!lyInhib) {
		lpl.Inhib.GiOrig = lpl.Inhib.Gi; // effective GiOrig
	}
}

// InhibFromPool computes inhibition Gi from Pool-level aggregated inhibition, including self and syn
void leabra::Layer::InhibFromPool(Context *ctx) {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Pool &pl = Pools[nrn.SubPool];
		Inhib.Self.Inhib(nrn.GiSelf, nrn.Act);
		nrn.Gi = pl.Inhib.Gi + nrn.GiSelf + nrn.GiSyn;
	}
}

// ActFromG computes rate-code activation from Ge, Gi, Gl conductances
// and updates learning running-average activations from that Act
void leabra::Layer::ActFromG(Context *ctx) {
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		Act.VmFromG(nrn);
		Act.ActFromG(nrn);
		Learn.AvgsFromAct(nrn);
	}
}

// AvgMaxAct computes the average and max Act stats, used in inhibition
void leabra::Layer::AvgMaxAct(Context *ctx) {
	for (Pool &pl: Pools) {
		pl.Inhib.Act.Init();
		for (int ni = pl.StIndex; ni < pl.EdIndex; ni++) {
			Neuron &nrn = Neurons[ni];
			if (nrn.IsOff()) {
				continue;
			}
			pl.Inhib.Act.UpdateValue(nrn.Act, ni);
		}
		pl.Inhib.Act.CalcAvg();
	}
}

void leabra::Layer::CyclePost(Context *ctx) {
}

// QuarterFinal does updating after end of quarter.
// Calls MinusPhase and PlusPhase for quarter = 2, 3.
void leabra::Layer::QuarterFinal(Context *ctx) {
	switch (ctx->Quarter) {
		case times::Q3:
			MinusPhase(ctx);
			return;
			break;
		case times::Q4:
			PlusPhase(ctx);
			return;
			break;
		default:
			break;
	}
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		switch (ctx->Quarter) {
			case 0:
				nrn.ActQ1 = nrn.Act;
				break;
			case 1:
				nrn.ActQ2 = nrn.Act;
				break;
			default:
				break;
		}
	}
}

// MinusPhase is called at the end of the minus phase (quarter 3), to record state.
void leabra::Layer::MinusPhase(Context *ctx) {
	for (Pool &pl: Pools) {
		pl.ActM = pl.Inhib.Act;
	}
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		nrn.ActM = nrn.Act;
		if (nrn.HasFlag(NeurHasTarg)) { // will be clamped in plus phase
			nrn.Ext = nrn.Targ;
			nrn.SetFlag(true, {NeurHasExt});
		}
	}
}

// PlusPhase is called at the end of the plus phase (quarter 4), to record state.
void leabra::Layer::PlusPhase(Context *ctx) {
	for (Pool &pl: Pools) {
		pl.ActP = pl.Inhib.Act;
	}
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		nrn.ActP = nrn.Act;
		nrn.ActDif = nrn.ActP - nrn.ActM;
		nrn.ActAvg += Act.Dt.AvgDt * (nrn.Act - nrn.ActAvg);
	}
	CosDiffFromActs();
}

// CosDiffFromActs computes the cosine difference in activation state between minus and plus phases.
// this is also used for modulating the amount of BCM hebbian learning
void leabra::Layer::CosDiffFromActs() {
	Pool &lpl = Pools[0];
	float avgM = lpl.ActM.Avg;
	float avgP = lpl.ActP.Avg;
	float cosv = 0;
	float ssm = 0;
	float ssp = 0;
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		float ap = nrn.ActP - avgP; // zero mean
		float am = nrn.ActM - avgM;
		cosv += ap * am;
		ssm += am * am;
		ssp += ap * ap;
	}

	float dist = std::sqrt(ssm * ssp);
	if (dist != 0) {
		cosv /= dist;
	}
	CosDiff.Cos = cosv;

	Learn.CosDiff.AvgVarFromCos(CosDiff.Avg, CosDiff.Var, CosDiff.Cos);

	if (IsTarget()) {
		CosDiff.AvgLrn = 0; // no BCM for non-hidden layers
		CosDiff.ModAvgLLrn = 0;
	} else {
		CosDiff.AvgLrn = 1 - CosDiff.Avg;
		CosDiff.ModAvgLLrn = Learn.AvgL.ErrModFromLayErr(CosDiff.AvgLrn);
	}
}

bool leabra::Layer::IsTarget() {
	return Type==LayerTypes::TargetLayer;
}

// DWt computes the weight change (learning) -- calls DWt method on sending pathways
void leabra::Layer::DWt() {
	for (Path &pt: SendPaths) {
		if (pt.Off) {
			continue;
		}
		pt.DWt();
	}
}

// WtFromDWt updates the weights from delta-weight changes -- on the sending pathways
void leabra::Layer::WtFromDWt() {
	for (Path &pt: SendPaths) {
		if (pt.Off) {
			continue;
		}
		pt.WtFromDWt();
	}
}

// WtBalFromWt computes the Weight Balance factors based on average recv weights
void leabra::Layer::WtBalFromWt() {
	for (Path &pt: RecvPaths) {
		if (pt.Off) {
			continue;
		}
		pt.WtBalFromWt();
	}
}

// LrateMult sets the new Lrate parameter for Paths to LrateInit * mult.
// Useful for implementing learning rate schedules.
void leabra::Layer::LrateMult(float mult) {
	for (Path &pt: RecvPaths) {
		// if p.Off { // keep all sync'd
		// 	continue
		// }
		pt.LrateMult(mult);
	}
}

// CostEst returns the estimated computational cost associated with this layer,
// separated by neuron-level and synapse-level, in arbitrary units where
// cost per synapse is 1.  Neuron-level computation is more expensive but
// there are typically many fewer neurons, so in larger networks, synaptic
// costs tend to dominate.  Neuron cost is estimated from TimerReport output
// for large networks.
std::tuple<int, int, int> leabra::Layer::CostEst() {
	int perNeur = 300; // cost per neuron, relative to synapse which is 1
	int neur = Neurons.size() * perNeur;
	int syn = 0;
	for (Path &pt: SendPaths) {
		int ns = pt.Syns.size();
		syn += ns;
	}
	int tot = neur + syn;
    return std::tuple<int, int, int>(neur, syn, tot);
}


std::tuple<int, int> leabra::Layer::MSE(float tol) {
	float sse, mse;
    int nn = Neurons.size();
	if (nn == 0) {
		return std::tuple<int,int>(0, 0);
	}
	sse = 0.0;
	for (Neuron &nrn: Neurons) {
		if (nrn.IsOff()) {
			continue;
		}
		float d;
		if (Type == CompareLayer) {
			d = nrn.Targ - nrn.ActM;
		} else {
			d = nrn.ActP - nrn.ActM;
		}
		if (std::abs(d) < tol) {
			continue;
		}
		sse += d * d;
	}
	mse = sse/nn;
	return std::tuple<int,int>(sse, mse);
}

// SSE returns the sum-squared-error over the layer, in terms of ActP - ActM
// (valid even on non-target layers FWIW).
// Uses the given tolerance per-unit to count an error at all
// (e.g., .5 = activity just has to be on the right side of .5).
// Use this in Python which only allows single return values.
float leabra::Layer::SSE(float tol) {
	auto intTuple = MSE(tol);
    return std::get<0>(intTuple);
}

// UnLesionNeurons unlesions (clears the Off flag) for all neurons in the layer
void leabra::Layer::UnLesionNeurons() {
	for (Neuron &nrn: Neurons) {
		nrn.SetFlag(false, {NeurOff});
	}
}

// LesionNeurons lesions (sets the Off flag) for given proportion (0-1) of neurons in layer
// returns number of neurons lesioned.  Emits error if prop > 1 as indication that percent
// might have been passed
int leabra::Layer::LesionNeurons(float prop) {
	UnLesionNeurons();
	if (prop > 1) {
		std::cerr << "LesionNeurons got a proportion > 1 -- must be 0-1 as *proportion* (not percent) of neurons to lesion: " << prop << std::endl;
		return 0;
	}
	int nn = Neurons.size();
	if (nn == 0) {
		return 0;
	}
	std::vector<int> p = rands::Perm(nn);
	int nl = int(prop * float(nn));
	for (int i = 0; i < nl; i++) {
		Neuron &nrn = Neurons[p[i]];
		nrn.SetFlag(true, {NeurOff});
	}
	return nl;
}

leabra::LayerShape::LayerShape(int x, int y, int poolsX, int poolsY): X(x),Y(y),PoolsX(poolsX),PoolsY(poolsY){}
