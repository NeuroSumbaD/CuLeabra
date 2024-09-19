#include "inhib.hpp"

inhib::SelfInhibParams::SelfInhibParams(bool On, float Gi, float Tau) {
    this->On = On;
    this->Gi = Gi;
    this->Tau = Tau;
    Update();
}

void inhib::SelfInhibParams::Update() {
    Dt = 1 / Tau;
}

// Inhib updates the self inhibition value based on current unit activation
void inhib::SelfInhibParams::Inhib(float *self, float act) {
    if (On) {
        *self += Dt * (Gi * act - *self);
    }
    else {
        *self = 0;
    }
}

std::string inhib::SelfInhibParams::StyleType() {
    return "SelfInhibParams";
}

std::string inhib::SelfInhibParams::StyleClass() {
    return "";
}

std::string inhib::SelfInhibParams::StyleName() {
    return "";
}

void inhib::SelfInhibParams::InitParamMaps() {
    ParamNameMap["On"] = (void *) &On;
    ParamNameMap["Gi"] = (void *) &Gi;
    ParamNameMap["Tau"] = (void *) &Tau;
    ParamNameMap["Dt"] = (void *) &Dt;

    ParamTypeMap["On"] = &typeid(bool);
    ParamTypeMap["Gi"] = &typeid(float);
    ParamTypeMap["Tau"] = &typeid(float);
    ParamTypeMap["Dt"] = &typeid(float);
}

inhib::ActAvgParams::ActAvgParams(float Init, bool Fixed, bool UseExtAct, bool Usefirst, float Tau, float Adjust) {
    this->Init = Init;
    this->Fixed = Fixed;
    this->UseExtAct = UseExtAct;
    this->UseFirst = UseFirst;
    Update();
}

void inhib::ActAvgParams::Update() {
    Dt = 1 / Tau;
}

// EffInit returns the initial value applied during InitWts for the AvgPAvgEff effective layer activity
float inhib::ActAvgParams::EffInit() {
    if (Fixed) {
		return Init;
	}
	return Adjust * Init;
}

// AvgFmAct updates the running-average activation given average activity level in layer
void inhib::ActAvgParams::AvgFmAct(float *avg, float act) {
    if (act < 0.0001) {
		return;
	}
	if (UseFirst && *avg == Init) {
		*avg += 0.5 * (act - *avg);
	} else {
		*avg += Dt * (act - *avg);
	}
}

// EffFmAvg updates the effective value from the running-average value
void inhib::ActAvgParams::EffFmAvg(float *eff, float avg) {
    if (Fixed) {
		*eff = Init;
	} else {
		*eff = Adjust * avg;
	}
}

std::string inhib::ActAvgParams::StyleType() {
    return "ActAvgParams";
}

std::string inhib::ActAvgParams::StyleClass() {
    return "";
}

std::string inhib::ActAvgParams::StyleName() {
    return "";
}

void inhib::ActAvgParams::InitParamMaps() {
    ParamNameMap["Init"] = (void*) &Init;
    ParamNameMap["Fixed"] = (void*) &Fixed;
    ParamNameMap["UseExtAct"] = (void*) &UseExtAct;
    ParamNameMap["UseFirst"] = (void*) &UseFirst;
    ParamNameMap["Tau"] = (void*) &Tau;
    ParamNameMap["Adjust"] = (void*) &Adjust;
    ParamNameMap["Dt"] = (void*) &Dt;

    ParamTypeMap["Init"] = &typeid(float);
    ParamTypeMap["Fixed"] = &typeid(bool);
    ParamTypeMap["UseExtAct"] = &typeid(bool);
    ParamTypeMap["UseFirst"] = &typeid(bool);
    ParamTypeMap["Tau"] = &typeid(float);
    ParamTypeMap["Adjust"] = &typeid(float);
    ParamTypeMap["Dt"] = &typeid(float);
}

inhib::InhibParams::InhibParams() {
    this->Layer = fffb::Params();
    this->Pool = fffb::Params();
    this->Self = inhib::SelfInhibParams();
    this->ActAvg = inhib::ActAvgParams();
}

std::string inhib::InhibParams::StyleType() {
    return "InhibParams";
}

std::string inhib::InhibParams::StyleClass() {
    return "";
}

std::string inhib::InhibParams::StyleName() {
    return "";
}

void inhib::InhibParams::InitParamMaps() {
    ParamNameMap["Layer"] = (void*) &Layer;
    ParamNameMap["Pool"] = (void*) &Pool;
    ParamNameMap["Self"] = (void*) &Self;
    ParamNameMap["ActAvg"] = (void*) &ActAvg;

    ParamTypeMap["Layer"] = &typeid(params::StylerObject);
    ParamTypeMap["Pool"] = &typeid(params::StylerObject);
    ParamTypeMap["Self"] = &typeid(params::StylerObject);
    ParamTypeMap["ActAvg"] = &typeid(params::StylerObject);
}
