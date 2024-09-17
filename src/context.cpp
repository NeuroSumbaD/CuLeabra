#include "context.hpp"

leabra::Context::Context(float timePerCyc, int cycPerQtr): TimePerCyc(timePerCyc), CycPerQtr(cycPerQtr) {
    Reset();
}

void leabra::Context::Reset(){ 
    Time=0;
    Cycle=0;
    CycleTot=0;
    Quarter=times::Quarters::Q1;
    PlusPhase=false;
}

void leabra::Context::AlphaCycStart(){
    Cycle=0; 
    Quarter=times::Quarters::Q1; 
    PlusPhase=false;
}

void leabra::Context::CycleInc(){
    Cycle++; 
    CycleTot++; 
    Time += TimePerCyc;
}

void leabra::Context::QuarterInc(){
    Quarter = times::NextQuarter(Quarter);
}

int leabra::Context::QuarterCycle(){
    return Cycle - ((int)Quarter * CycPerQtr);
}
