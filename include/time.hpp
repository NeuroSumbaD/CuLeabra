#pragma once

namespace time {
    enum Quarters {
        Q1,
        Q2,
        Q3,
        Q4
    };

    void Set(Quarters* Quart, int qtr);
    void Clear(Quarters* Quart, int qtr);
    bool Has(Quarters Quart, int qtr);
    bool HasNext(Quarters Quart, int qtr);
    bool HasPrev(Quarters Quart, int qtr);
    Quarters NextQuarter(Quarters Quart);

    enum TimeScales {
        Cycle,
        FastSpike,
        Quarter,
        Phase,
        BetaCycle,
        AlphaCycle,
        ThetaCycle,
        Event,
        Trial,
        Tick,
        Sequence,
        Condition,
        Block,
        Epoch,
        Run,
        Expt,
        Scene,
        Episode
    };

    struct Time {
        float time; // accumulated amount of time the network has been running, in simulation-time (not real world time), in seconds (NOTE LEABRA SPELLS THIS WITH CAPITAL T BUT THIS CONFLICTS WITH CONSTRUCTOR IN C++)
        int Cycle; // cycle counter: number of iterations of activation updating (settling) on the current alpha-cycle (100 msec / 10 Hz) trial -- this counts time sequentially through the entire trial, typically from 0 to 99 cycles
        int CycTot; // total cycle count -- this increments continuously from whenever it was last reset -- typically this is number of milliseconds in simulation time
        int Quarter; // [0-3] current gamma-frequency (25 msec / 40 Hz) quarter of alpha-cycle (100 msec / 10 Hz) trial being processed.  Due to 0-based indexing, the first quarter is 0, second is 1, etc -- the plus phase final quarter is 3.
        bool PlusPhase; // true if this is the plus phase (final quarter = 3) -- else minus phase
        float TimePerCyc; // [def: 0.001] amount of time to increment per cycle
        int CycPerQtr; // [def: 25] number of cycles per quarter to run -- 25 = standard 100 msec alpha-cycle
        Time(float TimePerCyc = 0.001, int CycPerQtr = 25);
        void Reset();
        void AlphaCycStart();
        void CycleInc();
        void QuarterInc();
        int QuarterCycle();
    };
    
}