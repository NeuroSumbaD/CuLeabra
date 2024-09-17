#pragma once
#include <string>

namespace minmax {

    // F32 represents a min / max range for float32 values.
    // Supports clipping, renormalizing, etc
    struct F32 {
        float Max;
        float Min;

        void Set(float mn, float mx);
        void SetInfinity();
        bool IsValid();
        bool InRange(float val);
        bool IsLow(float val);
        bool IsHigh(float val);
        float Range();
        float Scale();
        float Midpoint();
        float FitValInRange(float val);
        float NormValue(float val);
        float ProjValue(float val);
        float ClipValue(float val);
        float ClipNormValue(float val);
        std::string String();
        bool FitInRange(F32 oth);
    };
    
    // AvgMax holds average and max statistics
    struct AvgMax32 {
        float Avg;
        float Max;
        float Sum; // sum for computing average
        int MaxIndex; // index of max item
        int N; // number of items in sum
        int pad;
        int pad1;
        int pad2;
        AvgMax32();
        void UpdateValue(float val, int idx);
        void UpdateFromOther(int oSum, float oMax, int oN, int oMaxIndex);
        void CalcAvg();
        std::string String();
        void CopyFrom(AvgMax32* oth);

        void Init();
    };
    
}