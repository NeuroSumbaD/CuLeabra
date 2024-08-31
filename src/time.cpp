#include "time.hpp"
#include "bitflag.hpp"

time::Time::Time(float TimePerCyc, int CycPerQtr) {
    this->TimePerCyc = TimePerCyc;
    this->CycPerQtr = CycPerQtr;

    this->Cycle = 0;
    this->CycTot = 0;
    this->Quarter = 0;
    this->PlusPhase = false;
}

// Reset resets the counters all back to zero
void time::Time::Reset() {
    time = 0;
    Cycle = 0;
    CycTot = 0;
    Quarter = 0;
    PlusPhase = false;
    if (CycPerQtr == 0) {
        *this = Time(); // Reinitialize (double-check that this doesn't create dangling pointers...)
    }
}

// AlphaCycStart starts a new alpha-cycle (set of 4 quarters)
void time::Time::AlphaCycStart() {
    Cycle = 0;
    Quarter = 0;
}

// CycleInc increments at the cycle level
void time::Time::CycleInc() {
    Cycle++;
    CycTot++;
    time += TimePerCyc;
}

// QuarterInc increments at the quarter level, updating Quarter and PlusPhase
void time::Time::QuarterInc() {
    Quarter++;
    if (Quarter == 3) {
        PlusPhase = true;
    }
    else {
        PlusPhase = false;
    }
}

// QuarterCycle returns the number of cycles into current quarter
int time::Time::QuarterCycle() {
    float qmin = Quarter * CycPerQtr;
    return Cycle - qmin;
}

// Set sets given quarter bit (adds to any existing) (qtr = 0..3 = same as Quarters)
void time::Set(Quarters* Quart, int qtr) {
    bitflag::Set32((int*)Quart, qtr);
}

// Clear clears given quarter bit (qtr = 0..3 = same as Quarters)
void time::Clear(Quarters* Quart, int qtr) {
    bitflag::Clear32((int*)Quart, qtr);
}

// Has returns true if the given quarter is set (qtr = 0..3 = same as Quarters)
bool time::Has(Quarters Quart, int qtr) {
    return bitflag::Has32(Quart, qtr);
}

bool time::HasNext(Quarters Quart, int qtr) {
    int next = (qtr + 1) % 4;
    return bitflag::Has32(Quart, next);
}

bool time::HasPrev(Quarters Quart, int qtr) {
    int prev = (qtr - 1) % 4;
    return bitflag::Has32(Quart, prev);
}

time::Quarters time::NextQuarter(time::Quarters Quart) {
    switch (Quart) {
        case Q1:
            return Q2;
        case Q2:
            return Q3;
        case Q3:
            return Q4;
        case Q4:
            return Q1;
    }
}
