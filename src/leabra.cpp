#include "leabra.hpp"

void leabra::SelfInhibParams::Inhib(float &self, float act) {
    if (On){
        self += Dt * (Gi*act - self);
    } else {
        self = 0;
    }
}

float leabra::ActAvgParams::EffInit() {
    if (Fixed){
        return Init;
    } else {
        return Adjust * Init;
    }
}

void leabra::ActAvgParams::AvgFromAct(float &avg, float act) {
    if (act < 0.0001) {
        return ;
    };
    
    if (UseFirst && avg == Init) {
        avg += 0.5 * (act - avg);
    } else {
        avg += Dt * (act - avg);
    }
}

void leabra::ActAvgParams::EffFromAvg(float &eff, float avg) {
    if (Fixed){
        eff = Init;
    } else {
        eff = Adjust * avg;
    }
}

void leabra::InhibParams::Update() {
    Layer.Update();
    Pool.Update();
    Self.Update();
    ActAvg.Update();
}

void leabra::InhibParams::Defaults() {
    Layer.Defaults();
    Pool.Defaults();
    Self.Defaults();
    ActAvg.Defaults();
    // Note, each defaults function calls its own update (not needed here)
}
