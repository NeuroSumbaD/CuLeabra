#include "fffb.hpp"

fffb::Inhib::Inhib() {
    Zero();
    this->Ge = minmax::AvgMax32();
    this->Act = minmax::AvgMax32();
}

// Zero clears inhibition but does not affect Ge, Act averages
void fffb::Inhib::Zero() {
    FFi = 0;
    FBi = 0;
    Gi = 0;
    GiOrig = 0;
    LayGi = 0;
}

// Decay reduces inhibition values by given decay proportion
void fffb::Inhib::Decay(float decay) {
    Ge.Max -= decay * Ge.Max;
	Ge.Avg -= decay * Ge.Avg;
	Act.Max -= decay * Act.Max;
	Act.Avg -= decay * Act.Avg;
	FFi -= decay * FFi;
	FBi -= decay * FBi;
	Gi -= decay * Gi;
}

void fffb::Inhib::Init() {
    Zero();
	Ge.Init();
	Act.Init();
}

std::string fffb::Inhib::StyleType() {
    return "Inhib";
}

std::string fffb::Inhib::StyleClass() {
    return "";
}

std::string fffb::Inhib::StyleName() {
    return "";
}

void fffb::Inhib::InitParamMaps() {
    ParamNameMap["FFi"] = (void*) &FFi;
    ParamNameMap["FBi"] = (void*) &FBi;
    ParamNameMap["Gi"] = (void*) &Gi;
    ParamNameMap["GiOrig"] = (void*) &GiOrig;
    ParamNameMap["LayGi"] = (void*) &LayGi;

    ParamTypeMap["FFi"] = &typeid(float);
    ParamTypeMap["FBi"] = &typeid(float);
    ParamTypeMap["Gi"] = &typeid(float);
    ParamTypeMap["GiOrig"] = &typeid(float);
    ParamTypeMap["LayGi"] = &typeid(float);
}

fffb::Params::Params(float Gi, float FF, float FB, float FBTau, float MaxVsAvg, float FF0) {
    //TODO Check if On needs to be intitialized to zero or one
    this->Gi = Gi;
    this->FF = FF;
    this->FB = FB;
    this->FBTau = FBTau;
    this->MaxVsAvg = MaxVsAvg;
    this->FF0 = FF0;
    Update();
}

void fffb::Params::Update() {
    this->FBDt = 1 / this->FBTau;
}

void fffb::Params::Defaults(){
    Gi = 1.8;
    FF = 1;
    FB = 1;
    FBTau = 1.4;
    MaxVsAvg = 0;
    FF0 = 0.1;Update();
}

// FFInhib returns the feedforward inhibition value based on average and max excitatory conductance within
// relevant scope
float fffb::Params::FFInhib(float avgGe, float maxGe) {
    float ffNetin = avgGe + MaxVsAvg*(maxGe-avgGe);
	float ffi = 0;
	if (ffNetin > FF0) {
		ffi = FF * (ffNetin - FF0);
	}
	return ffi;
}

// FBInhib computes feedback inhibition value as function of average activation
float fffb::Params::FBInhib(float avgAct) {
    float fbi = FB * avgAct;
	return fbi;
}

// FBUpdt updates feedback inhibition using time-integration rate constant
void fffb::Params::FBUpdt(float *fbi, float newFbi) {
    *fbi += FBDt * (newFbi - *fbi);
}

// Inhib is full inhibition computation for given inhib state, which must have
// the Ge and Act values updated to reflect the current Avg and Max of those
// values in relevant inhibitory pool.
void fffb::Params::Inhib(fffb::Inhib *inh) {
    if (!On) {
		inh->Zero();
		return;
	}

	float ffi = FFInhib(inh->Ge.Avg, inh->Ge.Max);
	float fbi = FBInhib(inh->Act.Avg);

	inh->FFi = ffi;
	FBUpdt(&(inh->FBi), fbi);

	inh->Gi = Gi * (ffi + inh->FBi);
	inh->GiOrig = inh->Gi;
}

std::string fffb::Params::StyleType() {
    return "Params";
}

std::string fffb::Params::StyleClass() {
    return "";
}

std::string fffb::Params::StyleName() {
    return "";
}

void fffb::Params::InitParamMaps() {
    ParamNameMap["On"] = (void*) &On;
    ParamNameMap["Gi"] = (void*) &Gi;
    ParamNameMap["FF"] = (void*) &FF;
    ParamNameMap["FB"] = (void*) &FB;
    ParamNameMap["FBTau"] = (void*) &FBTau;
    ParamNameMap["MaxVsAvg"] = (void*) &MaxVsAvg;
    ParamNameMap["FF0"] = (void*) &FF0;
    ParamNameMap["FBDt"] = (void*) &FBDt;

    ParamTypeMap["On"] = &typeid(bool);
    ParamTypeMap["Gi"] = &typeid(float);
    ParamTypeMap["FF"] = &typeid(float);
    ParamTypeMap["FB"] = &typeid(float);
    ParamTypeMap["FBTau"] = &typeid(float);
    ParamTypeMap["MaxVsAvg"] = &typeid(float);
    ParamTypeMap["FF0"] = &typeid(float);
    ParamTypeMap["FBDt"] = &typeid(float);
}
