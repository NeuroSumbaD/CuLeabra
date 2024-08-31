#include "act.hpp"
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

void leabra::DtParams::GFmRaw(float geRaw, float &ge) {
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

leabra::ActNoiseParams::ActNoiseParams(ActNoiseType Type, bool Fixed) {
    this->Type = Type;
    this->Fixed = Fixed;
}

void leabra::WtInitParams::Defaults() {
    Mean = 0.5;
	Var = 0.25;
	Dist = std::uniform_real_distribution<float>(Mean-Var, Mean+Var);
	Sym = true;
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
