#include "knadapt.hpp"

knadapt::Chan::Chan(bool on, float rise, float max, float tau): On(on), Rise(rise), Max(max), Tau(tau) {
    Update();
}

void knadapt::Chan::Defaults()
{
    On = true;
    Rise = 0.01;
    Max = 0.1;
    Tau = 100;
    Update();
}

void knadapt::Chan::GcFmSpike(float *gKNa, bool spike) {
    if (On) {
		if (spike) {
			*gKNa += Rise * (Max - *gKNa);
		} else {
			*gKNa -= Dt * *gKNa;
		}
	} else {
		*gKNa = 0;
	}
}

void knadapt::Chan::GcFmRate(float *gKNa, float act) {
    if (On) {
		*gKNa += act*Rise*(Max-*gKNa) - (Dt * *gKNa);
	} else {
		*gKNa = 0;
	}
}

std::string knadapt::Chan::StyleType() {
    return "Chan";
}

std::string knadapt::Chan::StyleClass() {
    return "";
}

std::string knadapt::Chan::StyleName() {
    return "";
}

void knadapt::Chan::InitParamMaps() {
    ParamNameMap["On"] = (void*) &On;
    ParamNameMap["Rise"] = (void*) &Rise;
    ParamNameMap["Max"] = (void*) &Max;
    ParamNameMap["Tau"] = (void*) &Tau;
    ParamNameMap["Dt"] = (void*) &Dt;

    ParamTypeMap["On"] = &typeid(bool);
    ParamTypeMap["Rise"] = &typeid(float);
    ParamTypeMap["Max"] = &typeid(float);
    ParamTypeMap["Tau"] = &typeid(float);
    ParamTypeMap["Dt"] = &typeid(float);
}

knadapt::Params::Params(bool on, float rate) {
    On = on;
    Rate = rate;

    Fast = Chan(true, 0.05, 0.1, 50);

    Med = Chan(true, 0.02, 0.1, 200);
    
    Slow = Chan(true, 0.001, 1, 1000);

    // Update(); Redundant because Chan calls its own update
}

void knadapt::Params::Defaults() {
    On = true;
    Rate = 0.8;

    Fast = Chan();
    Fast.Tau = 50;
    Fast.Rise = 0.05;
    Fast.Max = 0.1;

    Med = Chan();
    Med.Tau = 200;
    Med.Rise = 0.02;
    Med.Max = 0.1;
    Slow = Chan();
    Slow.Tau = 1000;
    Slow.Rise = 0.001;
    Slow.Max = 1;

    Update();
}

void knadapt::Params::Update() {
    Fast.Update();
    Med.Update();
    Slow.Update();
}

void knadapt::Params::GcFromSpike(float *gKNaF, float *gKNaM, float *gKNaS, bool spike) {
    Fast.GcFmSpike(gKNaF, spike);
	Med.GcFmSpike(gKNaM, spike);
	Slow.GcFmSpike(gKNaS, spike);
}

void knadapt::Params::GcFromRate(float *gKNaF, float *gKNaM, float *gKNaS, float act) {
    Fast.GcFmSpike(gKNaF, act);
	Med.GcFmSpike(gKNaM, act);
	Slow.GcFmSpike(gKNaS, act);
}

std::string knadapt::Params::StyleType() {
    return "Params";
}

std::string knadapt::Params::StyleClass() {
    return "";
}

std::string knadapt::Params::StyleName() {
    return "";
}

void knadapt::Params::InitParamMaps(){
    ParamNameMap["On"] = (void*) & On;
    ParamNameMap["Rate"] = (void*) & Rate;
    ParamNameMap["Fast"] = (void*) & Fast;
    ParamNameMap["Med"] = (void*) & Med;
    ParamNameMap["Slow"] = (void*) & Slow;

    ParamTypeMap["On"] = &typeid(On);
    ParamTypeMap["Rate"] = &typeid(Rate);
    ParamTypeMap["Fast"] = &typeid(Fast);
    ParamTypeMap["Med"] = &typeid(Med);
    ParamTypeMap["Slow"] = &typeid(Slow);
}