#include "chans.hpp"

chans::Chans::Chans(float e, float l, float i, float k): E(e), L(l), I(i), K(k){}

// SetAll sets all the values
void chans::Chans::SetAll(float e, float l, float i, float k) {
    E = e;
    L = l;
    I = i;
    K = k;
}

// SetFmOtherMinus sets all the values from other Chans minus given value
void chans::Chans::SetFromOtherMinus(Chans oth, float minus) {
    E = oth.E - minus;
    L = oth.L - minus;
    I = oth.I - minus;
    K = oth.K - minus;
}

// SetFmMinusOther sets all the values from given value minus other Chans
void chans::Chans::SetFromMinusOther(float minus, Chans oth) {
    E = minus - oth.E;
    L = minus - oth.L;
    I = minus - oth.I;
    K = minus - oth.K;
}
