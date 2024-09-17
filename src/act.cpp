#include "act.hpp"

#include <vector>
#include <cmath>

leabra::OptThreshParams::OptThreshParams(float Send, float Delta) {
    this->Send = Send;
    this->Delta = Delta;
}

void leabra::OptThreshParams::Defaults() {
    Send = 0.1;
    Delta = 0.005;
}

leabra::ActInitParams::ActInitParams(float Decay, float Vm, float Act, float Ge) {
    this->Decay = Decay;
    this->Vm = Vm;
    this->Act = Act;
    this->Ge = Ge;
}

void leabra::ActInitParams::Defaults() {
    Decay = 1;
    Vm = 0.4;
	Act = 0;
	Ge = 0;
}

void leabra::ActInitParams::Update() {
}

leabra::DtParams::DtParams(float Integ, float VmTau, float AvgTau) {
    this->Integ = Integ;
    this->VmTau = VmTau;
    this->AvgTau = AvgTau;
    Update(); // Initialize derived member variables
}

void leabra::DtParams::Update() {
    this->VmDt = Integ / VmTau;
    this->GDt = Integ / GTau;
    this->AvgDt = 1 / AvgTau;
}

void leabra::DtParams::GFromRaw(float geRaw, float &ge) {
    ge += GDt * (geRaw - ge);
}

void leabra::DtParams::Defaults() {
    Integ = 1;
	VmTau = 3.3;
	GTau = 1.4;
	AvgTau = 200;
	Update();
}

leabra::ClampParams::ClampParams(bool Hard, float RangeMax, float Gain, bool Avg, float AvgGain) {
    this->Hard = Hard;
    this->Range.Max = RangeMax; // TODO: check if this Range gets automatically initialized
    this->Gain = Gain;
    this->Avg = Avg;
    this->AvgGain = AvgGain;
}

// AvgGe computes Avg-based Ge clamping value if using that option.
float leabra::ClampParams::AvgGe(float ext, float ge) {
    return AvgGain*Gain*ext + (1-AvgGain)*ge;
}

void leabra::ClampParams::Defaults() {
    Hard = true;
	Range.Max = 0.95;
	Gain = 0.2;
	Avg = false;
	AvgGain = 0.2;
}

void leabra::ClampParams::Update() {
}

leabra::ActNoiseParams::ActNoiseParams(){Type = ActNoiseType::NoNoise; Defaults();}

leabra::ActNoiseParams::ActNoiseParams(ActNoiseType Type, bool Fixed) {
    this->Type = Type;
    this->Fixed = Fixed;
}

void leabra::ActNoiseParams::Defaults(){Fixed = true;}

void leabra::ActNoiseParams::Update() {
}

leabra::WtInitParams::WtInitParams(float mean, float var, float par, rands::RandDists type):
	Dist(mean, var, par, type){

}

void leabra::WtInitParams::Defaults() {
    Mean = 0.5;
	Var = 0.25;
	DistType = rands::Uniform;
	Sym = true;
}

leabra::WtScaleParams::WtScaleParams(float abs, float rel): Abs(abs), Rel(rel) {
}

void leabra::WtScaleParams::Defaults(){Abs = 1; Rel = 1;}

void leabra::WtScaleParams::Update() {
}

float leabra::WtScaleParams::SLayActScale(float savg, float snu, float ncon) {
    ncon = std::max(ncon, 1.0f); // path Avg can be < 1 in some cases
	float semExtra = 2;
	int slayActN = int(std::round(savg * snu)); // sending layer actual # active
	slayActN = std::max(slayActN, 1);
	float sc;
	if (ncon == snu) {
		sc = 1 / slayActN;
	} else {
		int maxActN = int(std::min(ncon, float(slayActN))); // max number we could get
		int avgActN = int(std::round(savg * ncon));           // recv average actual # active if uniform
		avgActN = std::max(avgActN, 1);
		int expActN = avgActN + semExtra; // expected
		expActN = std::min(expActN, maxActN);
		sc = 1 / float(expActN);
	}
	return sc;
}

float leabra::WtScaleParams::FullScale(float savg, float snu, float ncon) {
    return Abs * Rel * SLayActScale(savg, snu, ncon);
}

leabra::ActParams::ActParams():
	XX1(), OptThresh(), Init(), Dt(), Gbar(1.0, 0.1, 1.0, 1.0), Erev(1.0, 0.3, 0.25, 0.25), Clamp(), Noise(), VmRange(), KNa(false), ErevSubThr(0,0,0,0), ThrSubErev(0,0,0,0) {
		VmRange.Max = 2.0;
		ErevSubThr.SetFromOtherMinus(Erev, XX1.Thr);
		ThrSubErev.SetFromMinusOther(XX1.Thr, Erev);
}

void leabra::ActParams::Defaults() {
    XX1.Defaults();
	OptThresh.Defaults();
	Init.Defaults();
	Dt.Defaults();
	Gbar.SetAll(1.0, 0.1, 1.0, 1.0);
	Erev.SetAll(1.0, 0.3, 0.25, 0.25);
	Clamp.Defaults();
	VmRange.Max = 2.0;
	KNa.Defaults();
	KNa.On = false;
	Noise.Defaults();
	Update();
}

void leabra::ActParams::Update() {
    ErevSubThr.SetFromOtherMinus(Erev, XX1.Thr);
	ThrSubErev.SetFromMinusOther(XX1.Thr, Erev);
    XX1.Update();
	OptThresh.Update();
	Init.Update();
	Dt.Update();
	Clamp.Update();
	Noise.Update();
	KNa.Update();
}

// InitGinc initializes the Ge excitatory and Gi inhibitory conductance accumulation states
// including ActSent and G*Raw values.
// called at start of trial always, and can be called optionally
// when delta-based Ge computation needs to be updated (e.g., weights
// might have changed strength)
void leabra::ActParams::InitGInc(Neuron &nrn) {
	nrn.ActSent = 0;
	nrn.GeRaw = 0;
	nrn.GiRaw = 0;
}

// InitActs initializes activation state in neuron -- called during InitWeights but otherwise not
// automatically called (DecayState is used instead)
void leabra::ActParams::InitActs(leabra::Neuron &nrn) {
	nrn.Act = Init.Act;
	nrn.ActLrn = Init.Act;
	nrn.Ge = Init.Ge;
	nrn.Gi = 0;
	nrn.Gk = 0;
	nrn.GknaFast = 0;
	nrn.GknaMed = 0;
	nrn.GknaSlow = 0;
	nrn.GiSelf = 0;
	nrn.GiSyn = 0;
	nrn.Inet = 0;
	nrn.Vm = Init.Vm;
	nrn.Targ = 0;
	nrn.Ext = 0;
	nrn.ActDel = 0;
	nrn.Spike = 0;
	nrn.ISI = -1;
	nrn.ISIAvg = -1;

	InitActQs(nrn);
	InitGInc(nrn);
}

// InitActQs initializes quarter-based activation states in neuron (ActQ0-2, ActM, ActP, ActDif)
// Called from InitActs, which is called from InitWeights, but otherwise not automatically called
// (DecayState is used instead)
void leabra::ActParams::InitActQs(leabra::Neuron &nrn) {
	nrn.ActQ0 = 0;
	nrn.ActQ1 = 0;
	nrn.ActQ2 = 0;
	nrn.ActM = 0;
	nrn.ActP = 0;
	nrn.ActDif = 0;
}

// GeFromRaw integrates Ge excitatory conductance from GeRaw value
// (can add other terms to geRaw prior to calling this)
void leabra::ActParams::GeFromRaw(Neuron &nrn, float geRaw) {
	if (!Clamp.Hard && nrn.HasFlag(NeurHasExt)) {
		if (Clamp.Avg) {
			geRaw = Clamp.AvgGe(nrn.Ext, geRaw);
		} else {
			geRaw += nrn.Ext * Clamp.Gain;
		}
	}

	Dt.GFromRaw(geRaw, nrn.Ge);
	// first place noise is required -- generate here!
	if (Noise.Type != NoNoise && !Noise.Fixed && Noise.DistType != rands::Mean) {
		nrn.Noise = Noise.Gen();
	}
	if (Noise.Type == GeNoise) {
		nrn.Ge += nrn.Noise;
	}
}

// GiFromRaw integrates GiSyn inhibitory synaptic conductance from GiRaw value
// (can add other terms to geRaw prior to calling this)
void leabra::ActParams::GiFromRaw(Neuron &nrn, float giRaw) {
	Dt.GFromRaw(giRaw, nrn.GiSyn);
	nrn.GiSyn = std::max(nrn.GiSyn, (float)0.0); // negative inhib G doesn't make any sense
}

// InetFromG computes net current from conductances and Vm
float leabra::ActParams::InetFromG(float vm, float ge, float gi, float gk) {
	return ge*(Erev.E-vm) + Gbar.L*(Erev.L-vm) + gi*(Erev.I-vm) + gk*(Erev.K-vm);
}

// VmFromG computes membrane potential Vm from conductances Ge, Gi, and Gk.
// The Vm value is only used in pure rate-code computation within the sub-threshold regime
// because firing rate is a direct function of excitatory conductance Ge.
void leabra::ActParams::VmFromG(Neuron &nrn) {
	float ge = nrn.Ge * Gbar.E;
	float gi = nrn.Gi * Gbar.I;
	float gk = nrn.Gk * Gbar.K;
	nrn.Inet = InetFromG(nrn.Vm, ge, gi, gk);
	float nwVm = nrn.Vm + Dt.VmDt*nrn.Inet;

	if (Noise.Type == VmNoise) {
		nwVm += nrn.Noise;
	}
	nrn.Vm = VmRange.ClipValue(nwVm);
}

// GeThrFromG computes the threshold for Ge based on all other conductances,
// including Gk.  This is used for computing the adapted Act value.
float leabra::ActParams::GeThrFromG(Neuron &nrn) {
    return ((Gbar.I*nrn.Gi*ErevSubThr.I + Gbar.L*ErevSubThr.L + Gbar.K*nrn.Gk*ErevSubThr.K) / ThrSubErev.E);
}

// GeThrFromGnoK computes the threshold for Ge based on other conductances,
// excluding Gk.  This is used for computing the non-adapted ActLrn value.
float leabra::ActParams::GeThrFromGnoK(Neuron &nrn) {
    return ((Gbar.I*nrn.Gi*ErevSubThr.I + Gbar.L*ErevSubThr.L) / ThrSubErev.E);
}

// ActFromG computes rate-coded activation Act from conductances Ge, Gi, Gk
void leabra::ActParams::ActFromG(Neuron &nrn) {
	if (HasHardClamp(nrn)) {
		HardClamp(nrn);
		return;
	}
	float nwAct, nwActLrn;
	if (nrn.Act < XX1.VmActThr && nrn.Vm <= XX1.Thr) {
		// note: this is quite important -- if you directly use the gelin
		// the whole time, then units are active right away -- need Vm dynamics to
		// drive subthreshold activation behavior
		nwAct = XX1.NoisyXX1(nrn.Vm - XX1.Thr);
		nwActLrn = nwAct;
	} else {
		float ge = nrn.Ge * Gbar.E;
		float geThr = GeThrFromG(nrn);
		nwAct = XX1.NoisyXX1(ge - geThr);
		geThr = GeThrFromGnoK(nrn);          // excludes K adaptation effect
		nwActLrn = XX1.NoisyXX1(ge - geThr); // learning is non-adapted
	}
	float &curAct = nrn.Act;
	nwAct = curAct + Dt.VmDt*(nwAct-curAct);
	nrn.ActDel = nwAct - curAct;

	if (Noise.Type == ActNoise) {
		nwAct += nrn.Noise;
	}
	nrn.Act = nwAct;

	nwActLrn = nrn.ActLrn + Dt.VmDt*(nwActLrn-nrn.ActLrn);
	nrn.ActLrn = nwActLrn;

	if (KNa.On) {
		KNa.GcFromRate(&nrn.GknaFast, &nrn.GknaMed, &nrn.GknaSlow, nrn.Act);
		nrn.Gk = nrn.GknaFast + nrn.GknaMed + nrn.GknaSlow;
	}
}

// HasHardClamp returns true if this neuron has external input that should be hard clamped
bool leabra::ActParams::HasHardClamp(Neuron &nrn) {
    return Clamp.Hard && nrn.HasFlag(NeurHasExt);
}

// DecayState decays the activation state toward initial values in proportion to given decay parameter
// Called with ac.Init.Decay by Layer during AlphaCycInit
void leabra::ActParams::DecayState(Neuron &nrn, float decay) {
	if (decay > 0) { // no-op for most, but not all..
		nrn.Act -= decay * (nrn.Act - Init.Act);
		nrn.Ge -= decay * (nrn.Ge - Init.Ge);
		nrn.Gi -= decay * nrn.Gi;
		nrn.GiSelf -= decay * nrn.GiSelf;
		nrn.Gk -= decay * nrn.Gk;
		nrn.Vm -= decay * (nrn.Vm - Init.Vm);
		nrn.GiSyn -= decay * nrn.GiSyn;
	}
	nrn.ActDel = 0;
	nrn.Inet = 0;
}

// HardClamp clamps activation from external input -- just does it -- use HasHardClamp to check
// if it should do it.  Also adds any Noise *if* noise is set to ActNoise.
void leabra::ActParams::HardClamp(Neuron &nrn) {
	float &ext = nrn.Ext;
	if (Noise.Type == ActNoise) {
		ext += nrn.Noise;
	}
	float clmp = Clamp.Range.ClipValue(ext);
	nrn.Act = clmp + nrn.Noise;
	nrn.ActLrn = clmp;
	nrn.Vm = XX1.Thr + nrn.Act/XX1.Gain;
	nrn.ActDel = 0;
	nrn.Inet = 0;
}
