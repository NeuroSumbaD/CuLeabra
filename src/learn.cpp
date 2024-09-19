#include "learn.hpp"
#include <cmath>

leabra::XCalParams::XCalParams(float mLrn, bool setLLrn, float lLrn, float dRev, float dThr, float lrnThr):
	MLrn(mLrn), SetLLrn(setLLrn), LLrn(lLrn), DRev(dRev), DThr(dThr), LrnThr(lrnThr){ 
	Update();
}

void leabra::XCalParams::Update()
{
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

std::string leabra::XCalParams::StyleType() {
    return "XCalParams";
}

std::string leabra::XCalParams::StyleClass() {
    return "";
}

std::string leabra::XCalParams::StyleName() {
    return "";
}

void leabra::XCalParams::InitParamMaps() {
	ParamNameMap["MLrn"] = (void*) &MLrn;
	ParamNameMap["SetLLrn"] = (void*) &SetLLrn;
	ParamNameMap["LLrn"] = (void*) &LLrn;
	ParamNameMap["DRev"] = (void*) &DRev;
	ParamNameMap["DThr"] = (void*) &DThr;
	ParamNameMap["LrnThr"] = (void*) &LrnThr;
	ParamNameMap["DRevRatio"] = (void*) &DRevRatio;

	ParamTypeMap["MLrn"] = &typeid(float);
	ParamTypeMap["SetLLrn"] = &typeid(bool);
	ParamTypeMap["LLrn"] = &typeid(float);
	ParamTypeMap["DRev"] = &typeid(float);
	ParamTypeMap["DThr"] = &typeid(float);
	ParamTypeMap["LrnThr"] = &typeid(float);
	ParamTypeMap["DRevRatio"] = &typeid(float);
}

leabra::LearnNeurParams::LearnNeurParams():ActAvg(), AvgL(), CosDiff(){}

void leabra::LearnNeurParams::Update()
{
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

std::string leabra::LearnNeurParams::StyleType() {
    return "LearnNeurParams";
}

std::string leabra::LearnNeurParams::StyleClass() {
    return "";
}

std::string leabra::LearnNeurParams::StyleName() {
    return "";
}

void leabra::LearnNeurParams::InitParamMaps() {
	ParamNameMap["ActAvg"] = (void*) &ActAvg;
	ParamNameMap["AvgL"] = (void*) &AvgL;
	ParamNameMap["CosDiff"] = (void*) &CosDiff;

	ParamTypeMap["ActAvg"] = &typeid(params::StylerObject);
	ParamTypeMap["AvgL"] = &typeid(params::StylerObject);
	ParamTypeMap["CosDiff"] = &typeid(params::StylerObject);
}

leabra::AvgLParams::AvgLParams(float init, float gain, float min, float tau, float lrnMax, float lrnMin, bool errMod, float modMin):
	Init(init),Gain(gain),Tau(tau),LrnMax(lrnMax),LrnMin(lrnMin),ErrMod(errMod),ModMin(modMin) {
	Update();
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

void leabra::AvgLParams::Update() {
	Dt=1/Tau;
	LrnFact=(LrnMax - LrnMin)/(Gain - Min);
}

std::string leabra::AvgLParams::StyleType() {
    return "AvgLParams";
}

std::string leabra::AvgLParams::StyleClass() {
    return "";
}

std::string leabra::AvgLParams::StyleName() {
    return "";
}

void leabra::AvgLParams::InitParamMaps() {
	ParamNameMap["Init"] = (void*) &Init;
	ParamNameMap["Gain"] = (void*) &Gain;
	ParamNameMap["Min"] = (void*) &Min;
	ParamNameMap["Tau"] = (void*) &Tau;
	ParamNameMap["LrnMax"] = (void*) &LrnMax;
	ParamNameMap["LrnMin"] = (void*) &LrnMin;
	ParamNameMap["ErrMod"] = (void*) &ErrMod;
	ParamNameMap["ModMin"] = (void*) &ModMin;
	ParamNameMap["Dt"] = (void*) &Dt;
	ParamNameMap["LrnFact"] = (void*) &LrnFact;

	ParamTypeMap["Init"] = &typeid(float);
	ParamTypeMap["Gain"] = &typeid(float);
	ParamTypeMap["Min"] = &typeid(float);
	ParamTypeMap["Tau"] = &typeid(float);
	ParamTypeMap["LrnMax"] = &typeid(float);
	ParamTypeMap["LrnMin"] = &typeid(float);
	ParamTypeMap["ErrMod"] = &typeid(bool);
	ParamTypeMap["ModMin"] = &typeid(float);
	ParamTypeMap["Dt"] = &typeid(float);
	ParamTypeMap["LrnFact"] = &typeid(float);
}

leabra::LrnActAvgParams::LrnActAvgParams(float SSTau, float STau, float MTau, float LrnM, float Init):
	SSTau(SSTau),STau(STau),MTau(MTau),LrnM(LrnM),Init(Init) {
	Update();
}

void leabra::LrnActAvgParams::AvgsFromAct(float ruAct, float &avgSS, float &avgS, float &avgM, float &avgSLrn)
{
    avgSS += SSDt * (ruAct = avgSS);
	avgS += SDt * (avgSS - avgS);
	avgM += MDt * (avgS - avgM);

	avgSLrn = LrnS * avgS + LrnM * avgM;
}

void leabra::LrnActAvgParams::Update(){
	SSDt = 1/SSTau;
	SDt=1/STau;
	MDt = 1/MTau;
	LrnS = 1 - LrnM;
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

std::string leabra::LrnActAvgParams::StyleType() {
    return "LrnActAvgParams";
}

std::string leabra::LrnActAvgParams::StyleClass() {
    return "";
}

std::string leabra::LrnActAvgParams::StyleName() {
    return "";
}

void leabra::LrnActAvgParams::InitParamMaps() {
	ParamNameMap["SSTau"] = (void*) &SSTau;
	ParamNameMap["STau"] = (void*) &STau;
	ParamNameMap["MTau"] = (void*) &MTau;
	ParamNameMap["LrnM"] = (void*) &LrnM;
	ParamNameMap["Init"] = (void*) &Init;
	ParamNameMap["SSDt"] = (void*) &SSDt;
	ParamNameMap["SDt"] = (void*) &SDt;
	ParamNameMap["MDt"] = (void*) &MDt;
	ParamNameMap["LrnS"] = (void*) &LrnS;

	ParamTypeMap["SSTau"] = &typeid(float);
	ParamTypeMap["STau"] = &typeid(float);
	ParamTypeMap["MTau"] = &typeid(float);
	ParamTypeMap["LrnM"] = &typeid(float);
	ParamTypeMap["Init"] = &typeid(float);
	ParamTypeMap["SSDt"] = &typeid(float);
	ParamTypeMap["SDt"] = &typeid(float);
	ParamTypeMap["MDt"] = &typeid(float);
	ParamTypeMap["LrnS"] = &typeid(float);
}

leabra::CosDiffParams::CosDiffParams(float tau):Tau(tau) {
	Update();
}

void leabra::CosDiffParams::AvgVarFromCos(float &avg, float &vr, float cos)
{
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

void leabra::CosDiffParams::Update() {
	Dt = 1/Tau;
	DtC = 1 - Dt;
}

void leabra::CosDiffParams::Defaults(){
	Tau = 100;
	Update();
}

std::string leabra::CosDiffParams::StyleType() {
    return "CosDiffParams";
}

std::string leabra::CosDiffParams::StyleClass() {
    return "";
}

std::string leabra::CosDiffParams::StyleName() {
    return "";
}

void leabra::CosDiffParams::InitParamMaps() {
	ParamNameMap["Tau"] = (void*) &Tau;
	ParamNameMap["Dt"] = (void*) &Dt;
	ParamNameMap["DtC"] = (void*) &DtC;

	ParamTypeMap["Tau"] = &typeid(float);
	ParamTypeMap["Dt"] = &typeid(float);
	ParamTypeMap["DtC"] = &typeid(float);
}

leabra::CosDiffStats::CosDiffStats() {
	Init();
}

void leabra::CosDiffStats::Init()
{
    Cos = 0;
	Avg = 0;
	Var = 0;
	AvgLrn = 0;
	ModAvgLLrn = 0;
}

std::string leabra::CosDiffStats::StyleType() {
    return "CosDiffStats";
}

std::string leabra::CosDiffStats::StyleClass() {
    return "";
}

std::string leabra::CosDiffStats::StyleName() {
    return "";
}

void leabra::CosDiffStats::InitParamMaps() {
	ParamNameMap["Cos"] = (void*) &Cos;
	ParamNameMap["Avg"] = (void*) &Avg;
	ParamNameMap["Var"] = (void*) &Var;
	ParamNameMap["AvgLrn"] = (void*) &AvgLrn;
	ParamNameMap["ModAvgLLrn"] = (void*) &ModAvgLLrn;

	ParamTypeMap["Cos"] = &typeid(float);
	ParamTypeMap["Avg"] = &typeid(float);
	ParamTypeMap["Var"] = &typeid(float);
	ParamTypeMap["AvgLrn"] = &typeid(float);
	ParamTypeMap["ModAvgLLrn"] = &typeid(float);
}

leabra::WtSigParams::WtSigParams(float gain, float off, bool softBound): Gain(gain), Off(off), SoftBound(softBound){
	Update();
}

void leabra::WtSigParams::Update() {
}

void leabra::WtSigParams::Defaults()
{
    Gain = 6, Off = 1;
	SoftBound = true;
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

std::string leabra::WtSigParams::StyleType() {
    return "WtSigParams";
}

std::string leabra::WtSigParams::StyleClass() {
    return "";
}

std::string leabra::WtSigParams::StyleName() {
    return "";
}

void leabra::WtSigParams::InitParamMaps() {
	ParamNameMap["Gain"] = (void*) &Gain;
	ParamNameMap["Off"] = (void*) &Off;
	ParamNameMap["SoftBound"] = (void*) &SoftBound;

	ParamTypeMap["Gain"] = &typeid(float);
	ParamTypeMap["Off"] = &typeid(float);
	ParamTypeMap["SoftBound"] = &typeid(bool);
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

leabra::DWtNormParams::DWtNormParams(bool on, float decayTau, float lrComp, float normMin, bool stats):
    On(on), DecayTau(decayTau), NormMin(normMin), LrComp(lrComp), Stats(stats) {
		Update();
}

void leabra::DWtNormParams::Update() {
    DecayDt = 1 / DecayTau;
	DecayDtC = 1 - DecayDt;
}

void leabra::DWtNormParams::Defaults() {
	On = true;
	DecayTau = 1000;
	LrComp = 0.15;
	NormMin = 0.001;
	Stats = false;
	Update();
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

std::string leabra::DWtNormParams::StyleType() {
    return "DWtNormParams";
}

std::string leabra::DWtNormParams::StyleClass() {
    return "";
}

std::string leabra::DWtNormParams::StyleName() {
    return "";
}

void leabra::DWtNormParams::InitParamMaps() {
	ParamNameMap["On"] = (void*) &On;
	ParamNameMap["DecayTau"] = (void*) &DecayTau;
	ParamNameMap["NormMin"] = (void*) &NormMin;
	ParamNameMap["LrComp"] = (void*) &LrComp;
	ParamNameMap["Stats"] = (void*) &Stats;
	ParamNameMap["DecayDt"] = (void*) &DecayDt;
	ParamNameMap["DecayDtC"] = (void*) &DecayDtC;

	ParamTypeMap["On"] = &typeid(bool);
	ParamTypeMap["DecayTau"] = &typeid(float);
	ParamTypeMap["NormMin"] = &typeid(float);
	ParamTypeMap["LrComp"] = &typeid(float);
	ParamTypeMap["Stats"] = &typeid(bool);
	ParamTypeMap["DecayDt"] = &typeid(float);
	ParamTypeMap["DecayDtC"] = &typeid(float);
}

leabra::WtBalParams::WtBalParams(float on, bool targs, float avgThr, float hiThr, float hiGain, float loThr, float loGain):
	On(on), Targs(targs), AvgThr(avgThr), HiThr(hiThr), HiGain(hiGain), LoThr(loThr), LoGain(loGain) {
}

void leabra::WtBalParams::Update() {
}

void leabra::WtBalParams::Defaults() {
	On = false;
	AvgThr = 0.25;
	HiThr = 0.4;
	HiGain = 4;
	LoThr = 0.4;
	LoGain = 6;
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

std::string leabra::WtBalParams::StyleType() {
    return "WtBalParams";
}

std::string leabra::WtBalParams::StyleClass() {
    return "";
}

std::string leabra::WtBalParams::StyleName() {
    return "";
}

void leabra::WtBalParams::InitParamMaps() {
	ParamNameMap["On"] = (void*) &On;
	ParamNameMap["Targs"] = (void*) &Targs;
	ParamNameMap["AvgThr"] = (void*) &AvgThr;
	ParamNameMap["HiThr"] = (void*) &HiThr;
	ParamNameMap["HiGain"] = (void*) &HiGain;
	ParamNameMap["LoThr"] = (void*) &LoThr;
	ParamNameMap["LoGain"] = (void*) &LoGain;

	ParamTypeMap["On"] = &typeid(bool);
	ParamTypeMap["Targs"] = &typeid(bool);
	ParamTypeMap["AvgThr"] = &typeid(float);
	ParamTypeMap["HiThr"] = &typeid(float);
	ParamTypeMap["HiGain"] = &typeid(float);
	ParamTypeMap["LoThr"] = &typeid(float);
	ParamTypeMap["LoGain"] = &typeid(float);
}

leabra::MomentumParams::MomentumParams(bool on, float mTau, float lrComp):
	On(on), MTau(mTau), LrComp(lrComp){
	Update();
}

void leabra::MomentumParams::Defaults(){
	On = true, MTau = 10;
	LrComp = 0.1;
	Update();
}

// MomentFromDWt updates synaptic moment variable based on dwt weight change value
// and returns new momentum factor * LrComp
float leabra::MomentumParams::MomentFromDWt(float &moment, float dwt) {
	moment = MDtC * moment + dwt;
	return LrComp * moment;
}

std::string leabra::MomentumParams::StyleType() {
    return "MomentumParams";
}

std::string leabra::MomentumParams::StyleClass() {
    return "";
}

std::string leabra::MomentumParams::StyleName() {
    return "";
}

void leabra::MomentumParams::InitParamMaps() {
	ParamNameMap["On"] = (void*) &On;
	ParamNameMap["MTau"] = (void*) &MTau;
	ParamNameMap["LrComp"] = (void*) &LrComp;
	ParamNameMap["MDt"] = (void*) &MDt;
	ParamNameMap["MDtC"] = (void*) &MDtC;

	ParamTypeMap["On"] = &typeid(bool);
	ParamTypeMap["MTau"] = &typeid(float);
	ParamTypeMap["LrComp"] = &typeid(float);
	ParamTypeMap["MDt"] = &typeid(float);
	ParamTypeMap["MDtC"] = &typeid(float);
}

leabra::LearnSynParams::LearnSynParams(bool learn, float lrate):
	Learn(learn), Lrate(lrate), LrateInit(lrate), XCal(), WtSig(), Norm(), Momentum(), WtBal() {}

void leabra::LearnSynParams::Update()
{
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

std::string leabra::LearnSynParams::StyleType() {
    return "LearnSynParams";
}

std::string leabra::LearnSynParams::StyleClass() {
    return "";
}

std::string leabra::LearnSynParams::StyleName() {
    return "";
}

void leabra::LearnSynParams::InitParamMaps() {
	ParamNameMap["Learn"] = (void*) &Learn;
	ParamNameMap["Lrate"] = (void*) &Lrate;
	ParamNameMap["LrateInit"] = (void*) &LrateInit;
	ParamNameMap["XCal"] = (void*) &XCal;
	ParamNameMap["WtSig"] = (void*) &WtSig;
	ParamNameMap["Norm"] = (void*) &Norm;
	ParamNameMap["Momentum"] = (void*) &Momentum;
	ParamNameMap["WtBal"] = (void*) &WtBal;

	ParamTypeMap["Learn"] = &typeid(bool);
	ParamTypeMap["Lrate"] = &typeid(float);
	ParamTypeMap["LrateInit"] = &typeid(float);
	ParamTypeMap["XCal"] = &typeid(params::StylerObject);
	ParamTypeMap["WtSig"] = &typeid(params::StylerObject);
	ParamTypeMap["Norm"] = &typeid(params::StylerObject);
	ParamTypeMap["Momentum"] = &typeid(params::StylerObject);
	ParamTypeMap["WtBal"] = &typeid(params::StylerObject);
}
