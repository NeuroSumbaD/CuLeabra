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
    tensor::Int32 *sendn = new tensor::Int32(send);
    tensor::Int32 *recvn = new tensor::Int32(send);
    tensor::Shape csh = tensor::AddShapes(recv, send);
    tensor::Bits *cons = new tensor::Bits(csh);
    return std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *>(sendn, recvn, cons);
}

void paths::GaussTopo::Defaults(){
    Sigma = 0.6;
    Wrap = true;
    CtrMove = 1;
}

bool paths::PoolTile::HasTopoWeights() {
    return GaussFull.On || GaussInPool.On || SigFull.On || SigInPool.On;
}

// TopoWeights sets values in given 4D or 6D tensor according to *Topo settings.
// wts is shaped with first 2 outer-most dims as Y, X of units within layer / pool
// of recv layer (these are units over which topography is defined)
// and remaing 2D or 4D is for receptive field Size by units within pool size for
// sending layer.
void paths::PoolTile::TopoWeights(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts) {
    if (GaussFull.On || GaussInPool.On) {
		if (send.NumDims() == 2) {
			return TopoWeightsGauss2D(send, recv, wts);
		} else {
			return TopoWeightsGauss4D(send, recv, wts);
		}
	}
	if (SigFull.On || SigInPool.On) {
		if (send.NumDims() == 2) {
			return TopoWeightsSigmoid2D(send, recv, wts);
		} else {
			return TopoWeightsSigmoid4D(send, recv, wts);
		}
	}
	throw std::invalid_argument("PoolTile:TopoWeights no Gauss or Sig params turned on");
	// log.Println(err)
	// return err
}

// TopoWeightsGauss2D sets values in given 4D tensor according to *Topo settings.
// wts is shaped with first 2 outer-most dims as Y, X of units within layer / pool
// of recv layer (these are units over which topography is defined)
// and remaing 2D is for sending layer size (2D = sender)
void paths::PoolTile::TopoWeightsGauss2D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts) {
    if (GaussFull.Sigma == 0) {
		GaussFull.Defaults();
	}
	if (GaussInPool.Sigma == 0) {
		GaussInPool.Defaults();
	}
	int sNuY = send.DimSize(0);
	int sNuX = send.DimSize(1);
	int rNuY = recv.DimSize(0); // ok if recv is 2D
	int rNuX = recv.DimSize(1);
	if (recv.NumDims() == 4) {
		rNuY = recv.DimSize(2);
		rNuX = recv.DimSize(3);
	}
	std::vector<int> wshp = {rNuY, rNuX, sNuY, sNuX};
	wts.SetShape(wshp, {"rNuY", "rNuX", "szY", "szX"});

	auto fsz = math::Vec2(float(sNuX-1), float(sNuY-1));    // full rf size
	math::Vec2 hfsz = fsz.MulScalar(0.5);                   // half rf
	float fsig = GaussFull.Sigma * hfsz.X;                  // full sigma
	if (fsig <= 0) {
		fsig = GaussFull.Sigma;
	}

	auto psz = math::Vec2(float(sNuX), float(sNuY)); // within-pool rf size
	if (sNuX > 1) {
		psz.X -= 1;
	}
	if (sNuY > 1) {
		psz.Y -= 1;
	}
	math::Vec2 hpsz = psz.MulScalar(0.5);            // half rf
	float psig = GaussInPool.Sigma * hpsz.X; // pool sigma
	if (psig <= 0) {
		psig = GaussInPool.Sigma;
	}

	auto rsz = math::Vec2(float(rNuX), float(rNuY)); // recv units-in-pool size
	if (rNuX > 1) {
		rsz.X -= 1;
	}
	if (rNuY > 1) {
		rsz.Y -= 1;
	}

	auto hrsz = rsz.MulScalar(0.5);
	for (int ruy = 0; ruy < rNuY; ruy++) {
		for (int rux = 0; rux < rNuX; rux++) {
			auto rpos = math::Vec2(float(rux), float(ruy)).Sub(hrsz).Div(hrsz); // -1..1 normalized r unit pos
			math::Vec2 rfpos = rpos.MulScalar(GaussFull.CtrMove);
			math::Vec2 rppos = rpos.MulScalar(GaussInPool.CtrMove);
			math::Vec2 sfctr = rfpos.Mul(hfsz).Add(hfsz); // sending center for full
			math::Vec2 spctr = rppos.Mul(hpsz).Add(hpsz); // sending center for within-pool
			for (int suy = 0; suy < sNuY; suy++) {
				for (int sux = 0; sux < sNuX; sux++) {
					float fwt = 1;
					if (GaussFull.On) {
						auto sf = math::Vec2(float(sux), float(suy));
						if (GaussFull.Wrap) {
							sf.X = math::WrapMinDist(sf.X, fsz.X, sfctr.X);
							sf.Y = math::WrapMinDist(sf.Y, fsz.Y, sfctr.Y);
						}
						fwt = math::GaussVecDistNoNorm(sf, sfctr, fsig);
					}
					float pwt = 1;
					if (GaussInPool.On) {
						auto sp = math::Vec2(float(sux), float(suy));
						if (GaussInPool.Wrap) {
							sp.X = math::WrapMinDist(sp.X, psz.X, spctr.X);
							sp.Y = math::WrapMinDist(sp.Y, psz.Y, spctr.Y);
						}
						pwt = math::GaussVecDistNoNorm(sp, spctr, psig);
					}
					float wt = fwt * pwt;
					float rwt = TopoRange.ProjValue(wt);
					wts.Set({ruy, rux, suy, sux}, rwt);
				}
			}
		}
	}
}

// TopoWeightsGauss4D sets values in given 6D tensor according to *Topo settings.
// wts is shaped with first 2 outer-most dims as Y, X of units within layer / pool
// of recv layer (these are units over which topography is defined)
// and remaing 4D is for receptive field Size by units within pool size for
// sending layer.
void paths::PoolTile::TopoWeightsGauss4D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts) {
    if (GaussFull.Sigma == 0) {
		GaussFull.Defaults();
	}
	if (GaussInPool.Sigma == 0) {
		GaussInPool.Defaults();
	}
	int sNuY = send.DimSize(2);
	int sNuX = send.DimSize(3);
	int rNuY = recv.DimSize(0); // ok if recv is 2D
	int rNuX = recv.DimSize(1);
	if (recv.NumDims() == 4) {
		rNuY = recv.DimSize(2);
		rNuX = recv.DimSize(3);
	}
	std::vector<int> wshp = {rNuY, rNuX, Size.Y, Size.X, sNuY, sNuX};
	wts.SetShape(wshp, {"rNuY", "rNuX", "szY", "szX", "sNuY", "sNuX"});

	auto fsz = math::Vec2(float(Size.X*sNuX-1), float(Size.Y*sNuY-1)); // full rf size
	math::Vec2 hfsz = fsz.MulScalar(0.5);                                               // half rf
	float fsig = GaussFull.Sigma * hfsz.X;                                      // full sigma
	if (fsig <= 0) {
		fsig = GaussFull.Sigma;
	}

	auto psz = math::Vec2(float(sNuX), float(sNuY)); // within-pool rf size
	if (sNuX > 1) {
		psz.X -= 1;
	}
	if (sNuY > 1) {
		psz.Y -=1;
	}
	math::Vec2 hpsz = psz.MulScalar(0.5);            // half rf
	float psig = GaussInPool.Sigma * hpsz.X; // pool sigma
	if (psig <= 0) {
		psig = GaussInPool.Sigma;
	}

	auto rsz = math::Vec2(float(rNuX), float(rNuY)); // recv units-in-pool size
	if (rNuX > 1) {
		rsz.X -= 1;
	}
	if (rNuY > 1) {
		rsz.Y -= 1;
	}
	math::Vec2 hrsz = rsz.MulScalar(0.5);
	for (int ruy = 0; ruy < rNuY; ruy++) {
		for (int rux = 0; rux < rNuX; rux++) {
			auto rpos = math::Vec2(float(rux), float(ruy)).Sub(hrsz).Div(hrsz); // -1..1 normalized r unit pos
			math::Vec2 rfpos = rpos.MulScalar(GaussFull.CtrMove);
			math::Vec2 rppos = rpos.MulScalar(GaussInPool.CtrMove);
			math::Vec2 sfctr = rfpos.Mul(hfsz).Add(hfsz); // sending center for full
			math::Vec2 spctr = rppos.Mul(hpsz).Add(hpsz); // sending center for within-pool
			for (int fy = 0; fy < Size.Y; fy++) {
				for (int fx = 0; fx < Size.X; fx++) {
					for (int suy = 0; suy < sNuY; suy++) {
						for (int sux = 0; sux < sNuX; sux++) {
							float fwt = 1;
							if (GaussFull.On) {
								auto sf = math::Vec2(float(fx*sNuX+sux), float(fy*sNuY+suy));
								if (GaussFull.Wrap) {
									sf.X = math::WrapMinDist(sf.X, fsz.X, sfctr.X);
									sf.Y = math::WrapMinDist(sf.Y, fsz.Y, sfctr.Y);
								}
								fwt = math::GaussVecDistNoNorm(sf, sfctr, fsig);
							}
							float pwt = float(1);
							if (GaussInPool.On) {
								auto sp = math::Vec2(float(sux), float(suy));
								if (GaussInPool.Wrap) {
									sp.X = math::WrapMinDist(sp.X, psz.X, spctr.X);
									sp.Y = math::WrapMinDist(sp.Y, psz.Y, spctr.Y);
								}
								pwt = math::GaussVecDistNoNorm(sp, spctr, psig);
							}
							float wt = fwt * pwt;
							float rwt = TopoRange.ProjValue(wt);
							wts.Set({ruy, rux, fy, fx, suy, sux}, rwt);
						}
					}
				}
			}
		}
	}
}

// TopoWeightsSigmoid2D sets values in given 4D tensor according to Topo settings.
// wts is shaped with first 2 outer-most dims as Y, X of units within pool
// of recv layer (these are units over which topography is defined)
// and remaing 2D is for sending layer (2D = sender).
void paths::PoolTile::TopoWeightsSigmoid2D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts) {
    if (SigFull.Gain == 0) {
		SigFull.Defaults();
	}
	if (SigInPool.Gain == 0) {
		SigInPool.Defaults();
	}
	int sNuY = send.DimSize(0);
	int sNuX = send.DimSize(1);
	int rNuY = recv.DimSize(0); // ok if recv is 2D
	int rNuX = recv.DimSize(1);
	if (recv.NumDims() == 4) {
		rNuY = recv.DimSize(2);
		rNuX = recv.DimSize(3);
	}
	std::vector<int> wshp = {rNuY, rNuX, sNuY, sNuX};
	wts.SetShape(wshp, {"rNuY", "rNuX", "sNuY", "sNuX"});

	auto fsz = math::Vec2(float(sNuX-1), float(sNuY-1)); // full rf size
	math::Vec2 hfsz = fsz.MulScalar(0.5);                           // half rf
	float fgain = SigFull.Gain * hfsz.X;                    // full gain

	auto psz = math::Vec2(float(sNuX), float(sNuY)); // within-pool rf size
	if (sNuX > 1) {
		psz.X -= 1;
	}
	if (sNuY > 1) {
		psz.Y -= 1;
	}
	math::Vec2 hpsz = psz.MulScalar(0.5);          // half rf
	float pgain = SigInPool.Gain * hpsz.X; // pool sigma

	auto rsz = math::Vec2(float(rNuX), float(rNuY)); // recv units-in-pool size
	if (rNuX > 1) {
		rsz.X -= 1;
	}
	if (rNuY > 1) {
		rsz.Y -= 1;
	}
	math::Vec2 hrsz = rsz.MulScalar(0.5);
	for (int ruy = 0; ruy < rNuY; ruy++) {
		for (int rux = 0; rux < rNuX; rux++) {
			math::Vec2 rpos = math::Vec2(float(rux), float(ruy)).Div(hrsz); // 0..2 normalized r unit pos
			math::Vec2 sgn = math::Vec2(1, 1);
			math::Vec2 rfpos = rpos.SubScalar(0.5).MulScalar(SigFull.CtrMove).AddScalar(0.5);
			math::Vec2 rppos = rpos.SubScalar(0.5).MulScalar(SigInPool.CtrMove).AddScalar(0.5);
			if (rpos.X >= 1) { // flip direction half-way through
				sgn.X = -1;
				rpos.X = -rpos.X + 1;
				rfpos.X = (rpos.X+0.5)*SigFull.CtrMove - 0.5;
				rppos.X = (rpos.X+0.5)*SigInPool.CtrMove - 0.5;
			}
			if (rpos.Y >= 1) {
				sgn.Y = -1;
				rpos.Y = -rpos.Y + 1;
				rfpos.Y = (rpos.Y+0.5)*SigFull.CtrMove - 0.5;
				rfpos.Y = (rpos.Y+0.5)*SigInPool.CtrMove - 0.5;
			}
			math::Vec2 sfctr = rfpos.Mul(fsz); // sending center for full
			math::Vec2 spctr = rppos.Mul(psz); // sending center for within-pool
			for (int suy = 0; suy < sNuY; suy++) {
				for (int sux = 0; sux < sNuX; sux++) {
					float fwt = 1;
					if (SigFull.On) {
						auto sf = math::Vec2(float(sux), float(suy));
						float sigx = math::Logistic(sgn.X*sf.X, fgain, sfctr.X);
						float sigy = math::Logistic(sgn.Y*sf.Y, fgain, sfctr.Y);
						fwt = sigx * sigy;
					}
					float pwt = 1;
					if (SigInPool.On) {
						math::Vec2 sp = math::Vec2(float(sux), float(suy));
						float sigx = math::Logistic(sgn.X*sp.X, pgain, spctr.X);
						float sigy = math::Logistic(sgn.Y*sp.Y, pgain, spctr.Y);
						float pwt = sigx * sigy;
					}
					float wt = fwt * pwt;
					float rwt = TopoRange.ProjValue(wt);
					wts.Set({ruy, rux, suy, sux}, rwt);
				}
			}
		}
	}
}

// TopoWeightsSigmoid4D sets values in given 6D tensor according to Topo settings.
// wts is shaped with first 2 outer-most dims as Y, X of units within pool
// of recv layer (these are units over which topography is defined)
// and remaing 2D is for receptive field Size by units within pool size for
// sending layer.
void paths::PoolTile::TopoWeightsSigmoid4D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts) {
    if (SigFull.Gain == 0) {
		SigFull.Defaults();
	}
	if (SigInPool.Gain == 0) {
		SigInPool.Defaults();
	}
	int sNuY = send.DimSize(2);
	int sNuX = send.DimSize(3);
	int rNuY = recv.DimSize(0); // ok if recv is 2D
	int rNuX = recv.DimSize(1);
	if (recv.NumDims() == 4) {
		rNuY = recv.DimSize(2);
		rNuX = recv.DimSize(3);
	}
	std::vector<int> wshp = {rNuY, rNuX, Size.Y, Size.X, sNuY, sNuX};
	wts.SetShape(wshp, {"rNuY", "rNuX", "szY", "szX", "sNuY", "sNuX"});

	auto fsz = math::Vec2(float(Size.X*sNuX-1), float(Size.Y*sNuY-1)); // full rf size
	math::Vec2 hfsz = fsz.MulScalar(0.5);                                               // half rf
	float fgain = SigFull.Gain * hfsz.X;                                        // full gain

	auto psz = math::Vec2(float(sNuX), float(sNuY)); // within-pool rf size
	if (sNuX > 1) {
		psz.X -= 1;
	}
	if (sNuY > 1) {
		psz.Y -= 1;
	}
	math::Vec2 hpsz = psz.MulScalar(0.5);          // half rf
	float pgain = SigInPool.Gain * hpsz.X; // pool sigma

	auto rsz = math::Vec2(float(rNuX), float(rNuY)); // recv units-in-pool size
	if (rNuX > 1) {
		rsz.X -= 1;
	}
	if (rNuY > 1) {
		rsz.Y -= 1;
	}
	auto hrsz = rsz.MulScalar(0.5);
	for (int ruy = 0; ruy < rNuY; ruy++) {
		for (int rux = 0; rux < rNuX; rux++) {
			auto rpos = math::Vec2(float(rux), float(ruy)).Div(hrsz); // 0..2 normalized r unit pos
			auto sgn = math::Vec2(1, 1);
			math::Vec2 rfpos = rpos.SubScalar(0.5).MulScalar(SigFull.CtrMove).AddScalar(0.5);
			math::Vec2 rppos = rpos.SubScalar(0.5).MulScalar(SigInPool.CtrMove).AddScalar(0.5);
			if (rpos.X >= 1) { // flip direction half-way through
				sgn.X = -1;
				rpos.X = -rpos.X + 1;
				rfpos.X = (rpos.X+0.5)*SigFull.CtrMove - 0.5;
				rppos.X = (rpos.X+0.5)*SigInPool.CtrMove - 0.5;
			}
			if (rpos.Y >= 1) {
				sgn.Y = -1;
				rpos.Y = -rpos.Y + 1;
				rfpos.Y = (rpos.Y+0.5)*SigFull.CtrMove - 0.5;
				rfpos.Y = (rpos.Y+0.5)*SigInPool.CtrMove - 0.5;
			}
			math::Vec2 sfctr = rfpos.Mul(fsz); // sending center for full
			math::Vec2 spctr = rppos.Mul(psz); // sending center for within-pool
			for (int fy = 0; fy < Size.Y; fy++) {
				for (int fx = 0; fx < Size.X; fx++) {
					for (int suy = 0; suy < sNuY; suy++) {
						for (int sux = 0; sux < sNuX; sux++) {
							float fwt = 1;
							if (SigFull.On) {
								auto sf = math::Vec2(float(fx*sNuX+sux), float(fy*sNuY+suy));
								float sigx = math::Logistic(sgn.X*sf.X, fgain, sfctr.X);
								float sigy = math::Logistic(sgn.Y*sf.Y, fgain, sfctr.Y);
								fwt = sigx * sigy;
							}
							float pwt = 1;
							if (SigInPool.On) {
								auto sp = math::Vec2(float(sux), float(suy));
								float sigx = math::Logistic(sgn.X*sp.X, pgain, spctr.X);
								float sigy = math::Logistic(sgn.Y*sp.Y, pgain, spctr.Y);
								pwt = sigx * sigy;
							}
							float wt = fwt * pwt;
							float rwt = TopoRange.ProjValue(wt);
							wts.Set({ruy, rux, fy, fx, suy, sux}, rwt);
						}
					}
				}
			}
		}
	}
}

void paths::SigmoidTopo::Defaults() {
    Gain = 0.05;
    CtrMove = 0.5;
}
