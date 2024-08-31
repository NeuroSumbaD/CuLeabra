#pragma once
#include "time.hpp"

namespace leabra {
    enum Modes {
        NoEvalMode,

        // AllModes indicates that the log should occur over all modes present in other items.
        AllModes,

        // Train is when the network is learning
        Train,

        // Test is when testing, typically without learning
        Test,

        // Validate is typically for a special held-out testing set
        Validate,

        // Analyze is when analyzing the representations and behavior of the network
        Analyze,

        // Debug is for recording info particularly useful for debugging
        Debug
    };
    
    struct Context {
        // accumulated amount of time the network has been running,
        // in simulation-time (not real world time), in seconds.
        float Time;

        // cycle counter: number of iterations of activation updating
        // (settling) on the current alpha-cycle (100 msec / 10 Hz) trial.
        // This counts time sequentially through the entire trial,
        // typically from 0 to 99 cycles.
        int Cycle;

        // total cycle count. this increments continuously from whenever
        // it was last reset, typically this is number of milliseconds
        // in simulation time.
        int CycleTot;

        // current gamma-frequency (25 msec / 40 Hz) quarter of alpha-cycle
        // (100 msec / 10 Hz) trial being processed.
        // Due to 0-based indexing, the first quarter is 0, second is 1, etc.
        // The plus phase final quarter is 3.
        time::Quarters Quarter;

        // true if this is the plus phase (final quarter = 3), else minus phase.
        bool PlusPhase;

        // amount of time to increment per cycle.
        float TimePerCyc; // default:"0.001

        // number of cycles per quarter to run: 25 = standard 100 msec alpha-cycle.
        int CycPerQtr; // default:"25"

        // current evaluation mode, e.g., Train, Test, etc
        Modes Mode;

        Context(float timePerCyc = 0.001, int cycPerQtr = 25): TimePerCyc(timePerCyc), CycPerQtr(cycPerQtr){Reset();};

        void Reset(){Time=0; Cycle=0; CycleTot=0; Quarter=time::Quarters::Q1; PlusPhase=false;};
        void AlphaCycStart(){Cycle=0; Quarter=time::Quarters::Q1; PlusPhase=false;};
        void CycleInc(){Cycle++; CycleTot++; Time += TimePerCyc;};
        void QuarterInc(){Quarter = time::NextQuarter(Quarter);};
        int QuarterCycle(){return Cycle - ((int)Quarter * CycPerQtr);};
    };
    
    
} // namespace leabra
