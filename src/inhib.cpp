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

inhib::InhibParams::InhibParams() {
    this->Layer = fffb::Params();
    this->Pool = fffb::Params();
    this->Self = inhib::SelfInhibParams();
    this->ActAvg = inhib::ActAvgParams();
}
