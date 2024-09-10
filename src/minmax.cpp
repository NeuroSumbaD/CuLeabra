#include "minmax.hpp"
#include <limits>

// const float MaxFloat32 = 3.402823466e+38;
// const float MinFloat32 =  1.175494351e-38

// Set sets the min and max values
void minmax::F32::Set(float mn, float mx) {
    Min = mn;
    Max = mx;
}

// SetInfinity sets the Min to +MaxFloat, Max to -MaxFloat -- suitable for
// iteratively calling Fit*InRange
void minmax::F32::SetInfinity() {
    Min = std::numeric_limits<float>::max();
    Max = -std::numeric_limits<float>::max();
}

// IsValid returns true if Min <= Max
bool minmax::F32::IsValid() {
    return Min <= Max;
}

// InRange tests whether value is within the range (>= Min and <= Max)
bool minmax::F32::InRange(float val) {
    return ((val >= Min) && (val <= Max));
}

// IsLow tests whether value is lower than the minimum
bool minmax::F32::IsLow(float val) {
    return val < Min;
}

// IsHigh tests whether value is higher than the maximum
bool minmax::F32::IsHigh(float val) {
    return val > Max;
}

// Range returns Max - Min
float minmax::F32::Range() {
    return Max - Min;
}

// Scale returns 1 / Range -- if Range = 0 then returns 0
float minmax::F32::Scale() {
    float r = Range();
    if (r != 0) {
        return 1.0/r;
    }
    return 0;
}

// Scale returns 1 / Range -- if Range = 0 then returns 0
float minmax::F32::Midpoint() {
    return 0.5 * (Max + Min);
}

// FitValInRange adjusts our Min, Max to fit given value within Min, Max range
// returns true if we had to adjust to fit.
float minmax::F32::FitValInRange(float val) {
    bool adj = false;
    if (val < Min) {
        Min = val;
        adj = true;
    }
    if (val > Max) {
        Max = val;
        adj = true;
    }
    return adj;
}

// NormVal normalizes value to 0-1 unit range relative to current Min / Max range
// Clips the value within Min-Max range first.
float minmax::F32::NormValue(float val) {
    return (ClipValue(val) - Min) * Scale();
}

// ProjVal projects a 0-1 normalized unit value into current Min / Max range (inverse of NormVal)
float minmax::F32::ProjValue(float val) {
    return Min + (val * Range());
}

// ClipVal clips given value within Min / Max range
// Note: a NaN will remain as a NaN
float minmax::F32::ClipValue(float val) {
    if (val < Min) {
		return Min;
	}
	if (val > Max) {
		return Max;
	}
	return val;
}

// ClipNormVal clips then normalizes given value within 0-1
// Note: a NaN will remain as a NaN
float minmax::F32::ClipNormValue(float val) {
    if (val < Min) {
		return 0;
	}
	if (val > Max) {
		return 1;
	}
	return NormValue(val);
}

std::string minmax::F32::String() {
    std::string out; //TODO: Check if it is better to use char * here
    std::sprintf(&out[0], "{%g %g}", Min, Max);
    return out;
}

// FitInRange adjusts our Min, Max to fit within those of other F32
// returns true if we had to adjust to fit.
bool minmax::F32::FitInRange(F32 oth) {
    bool adj = false;
	if (oth.Min < Min) {
		Min = oth.Min;
		adj = true;
	}
	if (oth.Max > Max) {
		Max = oth.Max;
		adj = true;
	}
	return adj;
}

minmax::AvgMax32::AvgMax32(){
    Avg = 0;
    Sum = 0;
    N = 0;
    Max = -std::numeric_limits<float>::max();
    MaxIndex = -1;
}

// UpdateVal updates stats from given value
void minmax::AvgMax32::UpdateValue(float val, int idx) {
    Sum += val;
	N++;
	if (val > Max) {
		Max = val;
		MaxIndex = idx;
	}
}

// UpdateFromOther updates these values from other AvgMax32 values
void minmax::AvgMax32::UpdateFromOther(int oSum, float oMax, int oN, int oMaxIndex) {
    Sum += oSum;
	N += oN;
	if (oMax > Max) {
		Max = oMax;
		MaxIndex = oMaxIndex;
	}
}

// CalcAvg computes the average given the current Sum and N values
void minmax::AvgMax32::CalcAvg() {
    if (N > 0) {
		Avg = Sum / N;
	} else {
		Avg = Sum;
		Max = Avg; // prevents Max from being -MaxFloat..
	}
}

std::string minmax::AvgMax32::String() {
    std::string out; //TODO: Check if it is better to use char * here
    std::sprintf(&out[0], "{Avg: %g, Max: %g, Sum: %g, MaxIndex: %d, N: %d}",
        Avg, Max, Sum, MaxIndex, N);
    return out;
}

void minmax::AvgMax32::CopyFrom(AvgMax32 *oth) {
    *this = *oth;
}

// Init initializes prior to new updates
void minmax::AvgMax32::Init() {
    Avg = 0;
	Sum = 0;
	N = 0;
	Max = -std::numeric_limits<float>::max();
	MaxIndex = -1;
}
