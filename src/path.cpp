#include "path.hpp"


std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *> paths::Full::Connect(tensor::Shape &send, tensor::Shape &recv, bool same){
    tensor::Int32 *sendn, *recvn;
    tensor::Bits *cons;
    
    auto tensorTuple = paths::NewTensors(send, recv);
    sendn = std::get<0>(tensorTuple);
    recvn = std::get<1>(tensorTuple);
    cons = std::get<2>(tensorTuple);

    cons->SetAll(true);

    int nsend = send.Len();
    int nrecv = recv.Len();

    if (same && !SelfCon) { // turn off all self connections
        for (int i = 0; i < nsend; i++) {
            int offset = i*nsend + i;
            cons->SetValue(offset, false);
        }
    }

    recvn->SetAll(nsend);
    sendn->SetAll(nrecv);
    // TODO: Investigate the purpose of these tensors... why do the elements carry redundant information?

    return std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *>(sendn, recvn, cons);
}

// NewTensors returns the tensors used for Connect method, based on layer sizes
std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *> paths::NewTensors(tensor::Shape &send, tensor::Shape &recv) {
    tensor::Int32 sendn(send);
    tensor::Int32 recvn(send);
    tensor::Shape csh = tensor::AddShapes(recv, send);
    tensor::Bits cons(csh);
    return std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *>(&sendn, &recvn, &cons);
}

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
void leabra::Path::Connect(Layer *slay, Layer *rlay, paths::Pattern *pat, PathTypes typ) {
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
