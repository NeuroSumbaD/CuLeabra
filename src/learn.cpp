#include "learn.hpp"
#include <cmath>

void leabra::XCalParams::Update() {
    if (DRev > 0) {
        DRevRatio = -(1 - DRev) / DRev;
    } else {
        DRevRatio = -1;
    }
}

void leabra::XCalParams::Defaults() {
	MLrn = 1;
	SetLLrn = false;
	LLrn = 1;
	DRev = 0.1;
	DThr = 0.0001;
	LrnThr = 0.01;
	Update();
}

// DWt is the XCAL function for weight change -- the "check mark" function -- no DGain, no ThrPMin
float leabra::XCalParams::DWt(float srval, float thrP) {
	float dwt;
	if (srval < DThr) {
		dwt = 0;
	} else if (srval > thrP * DRev) {
		dwt = (srval - thrP);
	} else {
		dwt = srval * DRevRatio;
	}
	return dwt;
}

// LongLrate returns the learning rate for long-term floating average component (BCM)
float leabra::XCalParams::LongLrate(float avgLLrn) {
	if (SetLLrn) {
		return LLrn;
	}
    return avgLLrn;
}

void leabra::LearnNeurParams::Update() {
	ActAvg.Update();
	AvgL.Update();
	CosDiff.Update();
}

void leabra::LearnNeurParams::Defaults() {
	ActAvg.Defaults();
	AvgL.Defaults();
	CosDiff.Defaults();
}

// AvgsFromAct updates the running averages based on current learning activation.
// Computed after new activation for current cycle is updated.
void leabra::LearnNeurParams::AvgsFromAct(Neuron &nrn) {
	ActAvg.AvgsFromAct(nrn.ActLrn, nrn.AvgSS, nrn.AvgS, nrn.AvgM, nrn.AvgSLrn);
}

// AvgLFromAvgM computes long-term average activation value, and learning factor, from current AvgM.
// Called at start of new alpha-cycle.
void leabra::LearnNeurParams::AvgLFromAvgM(Neuron &nrn) {
	AvgL.AvgLFromAvgM(nrn.AvgM, nrn.AvgL, nrn.AvgLLrn);
}

// InitActAvg initializes the running-average activation values that drive learning.
// Called by InitWeights (at start of learning).
void leabra::LearnNeurParams::InitActAvg(Neuron &nrn) {
	nrn.AvgSS = ActAvg.Init;
	nrn.AvgS = ActAvg.Init;
	nrn.AvgM = ActAvg.Init;
	nrn.AvgL = AvgL.Init;
	nrn.AvgSLrn = 0;
	nrn.ActAvg = ActAvg.Init;
}

// AvgLFromAvgM computes long-term average activation value, and learning factor, from given
// medium-scale running average activation avgM
void leabra::AvgLParams::AvgLFromAvgM(float avgM, float &avgL, float &lrn) {
	avgL += Dt * (Gain*avgM - avgL);
	if (avgL < Min) {
		avgL = Min;
	}
	lrn = LrnFact * (avgL - Min);
}

// ErrModFromLayErr computes AvgLLrn multiplier from layer cosine diff avg statistic
float leabra::AvgLParams::ErrModFromLayErr(float layCosDiffAvg) {
	float mod = 1;
	if (!ErrMod){
		return mod;
	}
	mod *= std::max(layCosDiffAvg, ModMin);
    return mod;
}

void leabra::AvgLParams::Defaults()
{
    Init=0.4;
	Gain=2.5;
	Min=0.2;
	Tau=10;
	LrnMax=0.5;
	LrnMin=0.0001;
	ErrMod=true;
	ModMin=0.01;
	Update();
}

void leabra::LrnActAvgParams::AvgsFromAct(float ruAct, float &avgSS, float &avgS, float &avgM, float &avgSLrn) {
	avgSS += SSDt * (ruAct = avgSS);
	avgS += SDt * (avgSS - avgS);
	avgM += MDt * (avgS - avgM);

	avgSLrn = LrnS * avgS + LrnM * avgM;
}

void leabra::LrnActAvgParams::Defaults()
{
    SSTau=2.0;
	STau=2.0;
	MTau=10.0;
	LrnM=0.1;
	Init=0.15;
	Update();
}

void leabra::CosDiffParams::AvgVarFromCos(float &avg, float &vr, float cos) {
	if (avg==0){
		avg = cos;
		vr = 0;
	} else {
		float del = cos - avg;
		float incr = Dt * del;
		avg += incr;
		if (vr == 0){
			vr = 2 * DtC * del * incr;
		} else {
			vr = DtC * (vr + del*incr);
		}
	}
}

void leabra::CosDiffStats::Init() {
	Cos = 0;
	Avg = 0;
	Var = 0;
	AvgLrn = 0;
	ModAvgLLrn = 0;
}

// SigFromLinWt returns sigmoidal contrast-enhanced weight from linear weight
float leabra::WtSigParams::SigFromLinWt(float lw) {
	if (Gain == 1 && Off == 1) {
		return lw;
	}
	if (Gain == 6 && Off == 1){
		return SigFun61(lw);
	}
    return SigFun(lw, Gain, Off);
}

// LinFromSigWt returns linear weight from sigmoidal contrast-enhanced weight
float leabra::WtSigParams::LinFromSigWt(float sw) {
	if (Gain == 1 && Off == 1) {
		return sw;
	}
	if (Gain == 6 && Off == 1){
		return SigInvFun61(sw);
	}
    return SigInvFun(sw, Gain, Off);
}

// SigFun is the sigmoid function for value w in 0-1 range, with gain and offset params
float leabra::SigFun(float w, float gain, float off) {
	if (w <= 0) {
		return 0;
	}
	if (w >= 1) {
		return 1;
	}
	return 1.0 / (1.0 + std::pow((off*(1-w))/w, gain));
}

// SigFun61 is the sigmoid function for value w in 0-1 range, with default gain = 6, offset = 1 params
float leabra::SigInvFun(float w, float gain, float off) {
	if (w <= 0) {
		return 0;
	}
	if (w >= 1) {
		return 1;
	}
	return 1.0 / (1.0 + std::pow((1.0-w)/w, 1/gain)/off);
}

// SigInvFun is the inverse of the sigmoid function
float leabra::SigFun61(float w) {
    if (w <= 0) {
		return 0;
	}
	if (w >= 1) {
		return 1;
	}
	float pw = (1 - w) / w;
	return 1.0 / (1.0 + pw*pw*pw*pw*pw*pw);
}

// SigInvFun61 is the inverse of the sigmoid function, with default gain = 6, offset = 1 params
float leabra::SigInvFun61(float w) {
	if (w <= 0) {
		return 0;
	}
	if (w >= 1) {
		return 1;
	}
    float rval = 1.0 / (1.0 + std::pow((1.0-w)/w, 1.0/6.0));
	return rval;
}

void leabra::DWtNormParams::Update() {
	DecayDt = 1 / DecayTau;
	DecayDtC = 1 - DecayDt;
}

// DWtNormParams updates the dwnorm running max_abs, slowly decaying value
// jumps up to max(abs_dwt) and slowly decays
// returns the effective normalization factor, as a multiplier, including lrate comp
float leabra::DWtNormParams::NormFromAbsDWt(float &norm, float absDwt) {
	norm = std::max(DecayDt*norm, absDwt);
	if (norm == 0) {
		return 1;
	}
    return LrComp / std::pow(norm, NormMin);
}

// WtBal computes weight balance factors for increase and decrease based on extent
// to which weights and average act exceed thresholds
std::tuple<float, float, float> leabra::WtBalParams::WtBal(float wbAvg) {
	float fact;
	float inc = 1;
	float dec = 1;
	if (wbAvg < LoThr) {
		if (wbAvg < AvgThr) {
			wbAvg = AvgThr; // prevent extreme low if everyone below thr
		}
		float fact = LoGain * (LoThr - wbAvg);
		dec = 1 / (1 + fact);
		inc = 2 - dec;
	} else if (wbAvg > HiThr) {
		fact = HiGain * (wbAvg - HiThr);
		inc = 1 / (1 + fact); // gets sigmoidally small toward 0 as fact gets larger -- is quick acting but saturates -- apply pressure earlier..
		dec = 2 - inc;        // as inc goes down, dec goes up..  sum to 2
	}

    return std::tuple<float, float, float>(fact, inc, dec);
}

// MomentFromDWt updates synaptic moment variable based on dwt weight change value
// and returns new momentum factor * LrComp
float leabra::MomentumParams::MomentFromDWt(float &moment, float dwt) {
	moment = MDtC * moment + dwt;
	return LrComp * moment;
}

void leabra::LearnSynParams::Update() {
	XCal.Update();
	WtSig.Update();
	Norm.Update();
	Momentum.Update();
	WtBal.Update();
}

void leabra::LearnSynParams::Defaults() {
	Learn = true;
	Lrate = 0.04;
	LrateInit = Lrate;
	XCal.Defaults();
	WtSig.Defaults();
	Norm.Defaults();
	Momentum.Defaults();
	WtBal.Defaults();
}

// LWtFromWt updates the linear weight value based on the current effective Wt value.
// effective weight is sigmoidally contrast-enhanced relative to the linear weight.
void leabra::LearnSynParams::LWtFromWt(Synapse &syn) {
	syn.LWt = WtSig.LinFromSigWt(syn.Wt / syn.Scale); // must factor out scale too! TODO: See if there is an optimization to remove this division
}

// WtFromLWt updates the effective weight value based on the current linear Wt value.
// effective weight is sigmoidally contrast-enhanced relative to the linear weight.
void leabra::LearnSynParams::WtFromLWt(Synapse &syn) {
	syn.Wt = WtSig.SigFromLinWt(syn.LWt);
	syn.Wt *= syn.Scale;
}

// CHLdWt returns the error-driven and BCM Hebbian weight change components for the
// temporally eXtended Contrastive Attractor Learning (XCAL), CHL version
std::tuple<float, float> leabra::LearnSynParams::CHLdWt(float suAvgSLrn, float suAvgM, float ruAvgSLrn, float ruAvgM, float ruAvgL) {
	float err, bcm;
	float srs = suAvgSLrn * ruAvgSLrn;
	float srm = suAvgM * ruAvgM;
	bcm = XCal.DWt(srs, ruAvgL);
	err = XCal.DWt(srs, srm);
    return std::tuple<float, float>(err, bcm);
}

// BCMdWt returns the BCM Hebbian weight change for AvgSLrn vs. AvgL
// long-term average floating activation on the receiver.
float leabra::LearnSynParams::BCMdWt(float suAvgSLrn, float ruAvgSLrn, float ruAvgL) {
	float srs = suAvgSLrn * ruAvgSLrn;
	return XCal.DWt(srs, ruAvgL);
}

// WtFromDWt updates the synaptic weights from accumulated weight changes
// wbInc and wbDec are the weight balance factors, wt is the sigmoidal contrast-enhanced
// weight and lwt is the linear weight value
void leabra::LearnSynParams::WtFromDWt(float wbInc, float wbDec, float &dwt, float &wt, float &lwt, float scale) {
		if (dwt == 0) {
			return;
		}
		if (WtSig.SoftBound) {
			if (dwt > 0) {
				dwt *= wbInc * (1 - lwt);
			} else {
				dwt *= wbDec * lwt;
			}
		} else {
			if (dwt > 0) {
				dwt *= wbInc;
			} else {
				dwt *= wbDec;
			}
		}
		lwt += dwt;
		if (lwt < 0) {
			lwt = 0;
		} else if (lwt > 1) {
			lwt = 1;
		}
		wt = scale * WtSig.SigFromLinWt(lwt);
		dwt = 0;
}
