#include "leabra.hpp"
#include "layer.hpp"

void leabra::SelfInhibParams::Inhib(float &self, float act) {
    if (On){
        self += Dt * (Gi*act - self);
    } else {
        self = 0;
    }
}

float leabra::ActAvgParams::EffInit() {
    if (Fixed){
        return Init;
    } else {
        return Adjust * Init;
    }
}

void leabra::ActAvgParams::AvgFromAct(float &avg, float act) {
    if (act < 0.0001) {
        return ;
    };
    
    if (UseFirst && avg == Init) {
        avg += 0.5 * (act - avg);
    } else {
        avg += Dt * (act - avg);
    }
}

void leabra::ActAvgParams::EffFromAvg(float &eff, float avg) {
    if (Fixed){
        eff = Init;
    } else {
        eff = Adjust * avg;
    }
}

void leabra::InhibParams::Update() {
    Layer.Update();
    Pool.Update();
    Self.Update();
    ActAvg.Update();
}

void leabra::InhibParams::Defaults() {
    Layer.Defaults();
    Pool.Defaults();
    Self.Defaults();
    ActAvg.Defaults();
    // Note, each defaults function calls its own update (not needed here)
}

leabra::Path::Path(std::string name, std::string cls):emer::Path(name, cls){Send=nullptr; Recv=nullptr;}

// UpdateParams updates all params given any changes that might have been made to individual values
void leabra::Path::UpdateParams() {
    WtScale.Update();
	Learn.Update();
	Learn.LrateInit = Learn.Lrate;
}


void leabra::Path::Defaults() {
    WtInit.Defaults();
	WtScale.Defaults();
	Learn.Defaults();
	GScale = 1;
}

int leabra::Path::NumSyns(){
	return Syns.size();
}

// SynIndex returns the index of the synapse between given send, recv unit indexes
// (1D, flat indexes). Returns -1 if synapse not found between these two neurons.
// Requires searching within connections for receiving unit.
int leabra::Path::SynIndex(int sidx, int ridx) {
    int nc = SConN[sidx];
	int st = SConIndexSt[sidx];
	for (int ci = 0; ci < nc; ci++) {
		int ri = SConIndex[st+ci];
		if (ri != ridx) {
			continue;
		}
		return st + ci;
	}
	return -1;
}

// Connect sets the connectivity between two layers and the pattern to use in interconnecting them
void leabra::Path::Connect(leabra::Layer *slay, leabra::Layer *rlay, paths::Pattern *pat, PathTypes typ) {
    Send = slay;
	Recv = rlay;
	Pattern = pat;
	Type = typ;
	Name = Send->Name + "To" + Recv->Name;
}

// Build constructs the full connectivity among the layers
// as specified in this pathway.
// Calls Validate and returns false if invalid.
// Pattern.Connect is called to get the pattern of the connection.
// Then the connection indexes are configured according to that pattern.
void leabra::Path::Build() {
    if (Off) {
        return;
    }
    // bool err = Validate(true);
    tensor::Shape &ssh = Send->Shape;
    tensor::Shape &rsh = Recv->Shape;

    std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *> tensorTuple = Pattern->Connect(ssh, rsh, Recv==Send);
    tensor::Int32 *sendn = std::get<0>(tensorTuple);
    tensor::Int32 *recvn = std::get<1>(tensorTuple);
    tensor::Bits *cons = std::get<2>(tensorTuple);

    int slen = ssh.Len();
    int rlen = rsh.Len();
    int tcons = SetNIndexSt(SConN, SConNAvgMax, SConIndexSt, *sendn);
    int tconr = SetNIndexSt(RConN, RConNAvgMax, RConIndexSt, *recvn);
    if (tconr != tcons) {
        std::cerr << String() << " programmer error: total recv cons " << tconr << " != total send cons " << tcons << std::endl;
	}
	RConIndex.reserve(tconr);
	RSynIndex.reserve(tconr);
	SConIndex.reserve(tcons);
    
    auto sconN = std::vector<int>(); // temporary mem needed to tracks cur n of sending cons
    sconN.reserve(slen);

    std::vector<bool> &cbits = cons->Values;
	for (int ri = 0; ri < rlen; ri++) {
		int rbi = ri * slen;     // recv bit index
		int rtcn = RConN[ri]; // number of cons
		int rst = RConIndexSt[ri];
		int rci = 0;
		for (int si = 0; si < slen; si++) {
			if (!cbits[rbi + si]) { // no connection
				continue;
			}
			int sst = SConIndexSt[si];
			if (rci >= rtcn) {
                std::cerr << String() << " programmer error: recv target total con number: " << rtcn << " exceeded at recv idx: " << ri << ", send idx: " << si << std::endl;
				break;
			}
			RConIndex[rst+rci] = si;

			int sci = sconN[si];
			int stcn = SConN[si];
			if (sci >= stcn) {
                std::cerr << String() << " programmer error: send target total con number: " << rtcn << " exceeded at recv idx: " << ri << ", send idx: " << si << std::endl;
				break;
			}
			SConIndex[sst+sci] = ri;
			RSynIndex[rst+rci] = sst + sci;
			(sconN[si])++;
			rci++;
		}
	}
	Syns.reserve(SConIndex.size());
	GInc.reserve(rlen);
	WbRecv.reserve(rlen);
}

// SetNIndexSt sets the *ConN and *ConIndexSt values given n tensor from Pat.
// Returns total number of connections for this direction.
int leabra::Path::SetNIndexSt(std::vector<int> &n, minmax::AvgMax32 &avgmax, std::vector<int> &idxst, tensor::Int32 &tn) {
    int ln = tn.Len();
	std::vector<int> &tnv = tn.Values;
	n.reserve(ln);
	idxst.reserve(ln);
	int idx = 0;
	// avgmax.Init();
	for (int i = 0; i < ln; i++) {
		int nv = tnv[i];
		n[i] = nv;
		idxst[i] = idx;
		idx += nv;
		avgmax.UpdateValue(nv, i);
	}
	avgmax.CalcAvg();
	return idx;
}

std::string leabra::Path::String() {
    std::string str = "";
	if (Recv == nullptr) {
		str += "recv=nullptr; ";
	} else {
		str += Recv->Name + " <- ";
	}
	if (Send == nullptr) {
		str += "send=nullptr";
	} else {
		str += Send->Name;
	}
	if (Pattern == nullptr) {
		str += " Pat=nullptr";
	} else {
		str += " Pat=" + Pattern->Name();
	}
	return str;
}

// SetScalesRPool initializes synaptic Scale values using given tensor
// of values which has unique values for each recv neuron within a given pool.
void leabra::Path::SetScalesRPool(tensor::Tensor<float> scales) {
    int rNuY = scales.Dim(0);
    int rNuX = scales.Dim(1);
    int rNu = rNuY * rNuX;
    float rfsz = scales.Len() / rNu;

    tensor::Shape rsh = Recv->Shape;
    int rNpY = rsh.Sizes[0];
    int rNpX = rsh.Sizes[1];
    bool r2d = false;

    if (rsh.NumDims() != 4) {
        r2d = true;
        rNpY = 1;
        rNpX = 1;
    }

    for (int rpy = 0; rpy < rNpY; rpy++) {
		for (int rpx = 0; rpx < rNpX; rpx++) {
			for (int ruy = 0; ruy < rNuY; ruy++) {
				for (int rux = 0; rux < rNuX; rux++) {
					int ri = 0;
					if (r2d) {
						ri = rsh.Offset({ruy, rux});
					} else {
						ri = rsh.Offset({rpy, rpx, ruy, rux});
					}
					float scst = (ruy*rNuX + rux) * rfsz;
					int nc = RConN[ri];
					int st = RConIndexSt[ri];
					for (int ci = 0; ci < nc; ci++) {
						// si := int(pj.RConIndex[st+ci]) // could verify coords etc
						int rsi = RSynIndex[st+ci];
						Synapse *sy = Syns[rsi];
						float sc = scales.Values[scst + ci];
						sy->Scale = sc;
					}
				}
			}
		}
	}
}

// SetWtsFunc initializes synaptic Wt value using given function
// based on receiving and sending unit indexes.
void leabra::Path::SetWtsFunc(std::function<float(int si, int ri, tensor::Shape &send, tensor::Shape &recv)> wtFun) {
	tensor::Shape &rsh = Recv->Shape;
	int rn = rsh.Len();
	tensor::Shape &ssh = Send->Shape;

	for (int ri = 0; ri < rn; ri++) {
		int nc = RConN[ri];
		int st = RConIndexSt[ri];
		for (int ci = 0; ci < nc; ci++) {
			int si = RConIndex[st+ci];
			float wt = wtFun(si, ri, ssh, rsh);
			int rsi = RSynIndex[st+ci];
			Synapse *sy = Syns[rsi];
			sy->Wt = wt * sy->Scale;
			Learn.LWtFromWt(*sy);
		}
	}
}

// SetScalesFunc initializes synaptic Scale values using given function
// based on receiving and sending unit indexes.
void leabra::Path::SetScalesFunc(std::function<float(int si, int ri, tensor::Shape &send, tensor::Shape &recv)> scaleFun) {
	tensor::Shape &rsh = Recv->Shape;
	int rn = rsh.Len();
	tensor::Shape &ssh = Send->Shape;

	for (int ri = 0; ri < rn; ri++) {
		int nc = RConN[ri];
		int st = RConIndexSt[ri];
		for (int ci = 0; ci < nc; ci++) {
			int si = RConIndex[st+ci];
			float sc = scaleFun(si, ri, ssh, rsh);
			int rsi = RSynIndex[st+ci];
			Synapse *sy = Syns[rsi];
			sy->Scale = sc;
		}
	}
}

// InitWeightsSyn initializes weight values based on WtInit randomness parameters
// for an individual synapse.
// It also updates the linear weight value based on the sigmoidal weight value.
void leabra::Path::InitWeightsSyn(Synapse &syn) {
	if (syn.Scale == 0) {
		syn.Scale = 1;
	}
	syn.Wt = WtInit.Gen();
	// enforce normalized weight range -- required for most uses and if not
	// then a new type of path should be used:
	if (syn.Wt < 0) {
		syn.Wt = 0;
	}
	if (syn.Wt > 1) {
		syn.Wt = 1;
	}
	syn.LWt = Learn.WtSig.LinFromSigWt(syn.Wt);
	syn.Wt *= syn.Scale; // note: scale comes after so LWt is always "pure" non-scaled value
	syn.DWt = 0;
	syn.Norm = 0;
	syn.Moment = 0;
}

// InitWeights initializes weight values according to Learn.WtInit params
void leabra::Path::InitWeights() {
	for (Synapse *sy: Syns) {
		InitWeightsSyn(*sy);
	}
	for (WtBalRecvPath &wb: WbRecv) {
		wb.Init();
	}
	InitGInc();
}

// InitWtSym initializes weight symmetry -- is given the reciprocal pathway where
// the Send and Recv layers are reversed.
void leabra::Path::InitWtSym(Path &rpt) {
	leabra::Layer &slay = *Send;
	int ns = slay.Neurons.size();
	for (int si = 0; si < ns; si++) {
		int nc = SConN[si];
		int st = SConIndexSt[si];
		for (int ci = 0; ci < nc; ci++) {
			Synapse &sy = *Syns[st+ci];
			int ri = SConIndex[st+ci];
			// now we need to find the reciprocal synapse on rpt!
			// look in ri for sending connections
			int rsi = ri;
			if (rpt.SConN.size() == 0) {
				continue;
			}
			int rsnc = rpt.SConN[rsi];
			if (rsnc == 0) {
				continue;
			}
			int rsst = rpt.SConIndexSt[rsi];
			int rist = rpt.SConIndex[rsst]; // starting index in recv path
			int ried = rpt.SConIndex[rsst+rsnc-1]; // ending index
			if (si < rist || si > ried) {        // fast reject -- paths are always in order!
				continue;
			}
			// start at index proportional to si relative to rist
			int up = 0;
			if (ried > rist) {
				up = int(float(rsnc) * float(si-rist) / float(ried-rist));
			}
			int dn = up - 1;

			for (;;) {
				bool doing = false;
				if (up < rsnc) {
					doing = true;
					int rrii = rsst + up;
					int rri = rpt.SConIndex[rrii];
					if (rri == si) {
						Synapse &rsy = *Syns[rrii];
						rsy.Wt = sy.Wt;
						rsy.LWt = sy.LWt;
						rsy.Scale = sy.Scale;
						// note: if we support SymFromTop then can have option to go other way
						break;
					}
					up++;
				}
				if (dn >= 0) {
					doing = true;
					int rrii = rsst + dn;
					int rri = rpt.SConIndex[rrii];
					if (rri == si) {
						Synapse &rsy = *Syns[rrii];
						rsy.Wt = sy.Wt;
						rsy.LWt = sy.LWt;
						rsy.Scale = sy.Scale;
						// note: if we support SymFromTop then can have option to go other way
						break;
					}
					dn--;
				}
				if (!doing) {
					break;
				}
			}
		}
	}
}

// InitGInc initializes the per-pathway GInc threadsafe increment -- not
// typically needed (called during InitWeights only) but can be called when needed
void leabra::Path::InitGInc() {
	for (float &ginc: GInc) {
		ginc = 0;
	}
}


void leabra::Path::SendGDelta(int si, float delta){
	float scdel = delta * GScale;
	int nc = SConN[si];
	int st = SConIndexSt[si];

	// slicing is ugly in c++...
	auto synStart = Syns.begin() + st;
	auto synEnd = Syns.begin() + st + nc;
	auto syns = std::vector<leabra::Synapse *>(synStart, synEnd);

	auto sconStart = SConIndex.begin() + st;
	auto sconEnd = SConIndex.begin() + st + nc;
	auto scons = std::vector<int>(sconStart, sconEnd);

	for (uint ci = 0; ci < syns.size();  ci++) {
		int ri = scons[ci];
		GInc[ri] += scdel * syns[ci]->Wt;
	}
}

// RecvGInc increments the receiver's GeRaw or GiRaw from that of all the pathways.
void leabra::Path::RecvGInc() {
	Layer &rlay = *Recv;
	if (Type == InhibPath) {
		for (uint ri = 0; ri < rlay.Neurons.size(); ri++) {
			Neuron &rn = rlay.Neurons[ri];
			rn.GiRaw += GInc[ri];
			GInc[ri] = 0;
		}
	} else {
		for (uint ri = 0; ri < rlay.Neurons.size(); ri++) {
			Neuron &rn = rlay.Neurons[ri];
			rn.GeRaw += GInc[ri];
			GInc[ri] = 0;
		}
	}
}

// DWt computes the weight change (learning) -- on sending pathways
void leabra::Path::DWt() {
	if (!Learn.Learn) {
		return;
	}
	Layer &slay = *Send;
	Layer &rlay = *Recv;
	for (uint si = 0; si < slay.Neurons.size(); si++) {
		Neuron &sn = slay.Neurons[si];
		if (sn.AvgS < Learn.XCal.LrnThr && sn.AvgM < Learn.XCal.LrnThr) {
			continue;
		}
		int nc = int(SConN[si]);
		int st = int(SConIndexSt[si]);

		auto start = Syns.begin() + st;
		auto end = Syns.begin() + st + nc;
		auto syns = std::vector<leabra::Synapse *>(start, end);

		auto sconStart = SConIndex.begin() + st;
		auto sconEnd = SConIndex.begin() + st + nc;
		auto scons = std::vector<int>(sconStart, sconEnd);

		for (uint ci = 0; ci < syns.size(); ci++) {
			Synapse &sy = *syns[ci];
			int ri = scons[ci];
			Neuron &rn = rlay.Neurons[ri];
			float err, bcm;
			auto dwtTuple = Learn.CHLdWt(sn.AvgSLrn, sn.AvgM, rn.AvgSLrn, rn.AvgM, rn.AvgL);
			err = std::get<0>(dwtTuple);
			bcm = std::get<1>(dwtTuple);

			bcm *= Learn.XCal.LongLrate(rn.AvgLLrn);
			err *= Learn.XCal.MLrn;
			float dwt = bcm + err;
			float norm = 1;
			if (Learn.Norm.On) {
				norm = Learn.Norm.NormFromAbsDWt(sy.Norm, std::abs(dwt));
			}
			if (Learn.Momentum.On) {
				dwt = norm * Learn.Momentum.MomentFromDWt(sy.Moment, dwt);
			} else {
				dwt *= norm;
			}
			sy.DWt += Learn.Lrate * dwt;
		}
		// aggregate max DWtNorm over sending synapses
		if (Learn.Norm.On) {
			float maxNorm = 0;
			for (uint ci = 0; ci < syns.size(); ci++) {
				Synapse &sy = *syns[ci];
				if (sy.Norm > maxNorm) {
					maxNorm = sy.Norm;
				}
			}
			for (uint ci = 0; ci < syns.size(); ci++) {
				Synapse &sy = *syns[ci];
				sy.Norm = maxNorm;
			}
		}
	}
}

// WtFromDWt updates the synaptic weight values from delta-weight changes -- on sending pathways
void leabra::Path::WtFromDWt() {
	if (!Learn.Learn) {
		return;
	}
	if (Learn.WtBal.On) {
		for (uint si = 0; si < Syns.size(); si++) {
			Synapse &sy = *Syns[si];
			int ri = SConIndex[si];
			WtBalRecvPath &wb = WbRecv[ri];
			Learn.WtFromDWt(wb.Inc, wb.Dec, sy.DWt, sy.Wt, sy.LWt, sy.Scale);
		}
	} else {
		for (uint si = 0; si < Syns.size(); si++) {
			Synapse &sy = *Syns[si];
			Learn.WtFromDWt(1, 1, sy.DWt, sy.Wt, sy.LWt, sy.Scale);
		}
	}
}

// WtBalFromWt computes the Weight Balance factors based on average recv weights
void leabra::Path::WtBalFromWt() {
	if (!Learn.Learn || !Learn.WtBal.On) {
		return;
	}

	leabra::Layer &rlay = *Recv;
	if (!Learn.WtBal.Targs && rlay.IsTarget()) {
		return;
	}
	for (uint ri = 0; ri < rlay.Neurons.size(); ri++) {
		int nc = RConN[ri];
		if (nc < 1) {
			continue;
		}
		WtBalRecvPath &wb = WbRecv[ri];
		int st = RConIndexSt[ri];

		auto start = RSynIndex.begin() + st;
		auto end = RSynIndex.begin() + st + nc;
		auto rsidxs = std::vector<int>(start, end);

		float sumWt = 0;
		int sumN = 0;
		for (uint ci = 0; ci < rsidxs.size(); ci++) {
			int rsi = rsidxs[ci];
			Synapse &sy = *Syns[rsi];
			if (sy.Wt >= Learn.WtBal.AvgThr) {
				sumWt += sy.Wt;
				sumN++;
			}
		}
		if (sumN > 0) {
			sumWt /= float(sumN);
		} else {
			sumWt = 0;
		}
		wb.Avg = sumWt;
		std::tuple<float, float, float> wtbalTuple = Learn.WtBal.WtBal(sumWt);
		wb.Fact = std::get<0>(wtbalTuple);
		wb.Inc = std::get<1>(wtbalTuple);
		wb.Dec = std::get<2>(wtbalTuple);
	}
}

// LrateMult sets the new Lrate parameter for Paths to LrateInit * mult.
// Useful for implementing learning rate schedules.
void leabra::Path::LrateMult(float mult) {
	Learn.Lrate = Learn.LrateInit * mult;
}

std::string PathTypeArr[] = {"ForwardPath","BackPath","LateralPath","InhibPath","CTCtxtPath"};

std::string leabra::Path::TypeName(){
	return PathTypeArr[Type];
}

emer::Layer *leabra::Path::SendLayer() {
	return Send;
}

emer::Layer *leabra::Path::RecvLayer() {
	return Recv;
}
