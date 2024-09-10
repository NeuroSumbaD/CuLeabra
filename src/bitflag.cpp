#include "bitflag.hpp"
#include <iostream>
#include <cstdarg>

void bitflag::SetMask32(int *bits, int mask) {
    *bits |= mask;
}

// Base case for recursive definition below
int bitflag::Mask32(int flag) {
    return (1 << flag);
}

void bitflag::ClearMask32(int* bits, int mask) {
    *bits &= ~mask;
}

// SetFlag sets the value of the given flags in these flags to the given value.
void bitflag::Set32(int *bits, bool on, std::vector<int> flags) {
    // *bits |= mask;
    int mask = 0;
    for (int &v: flags) {
        mask |= 1 << v;
    }
    if (on) {
        *bits |= mask;
    } else {
        *bits &= ~mask;
    }
}

bool bitflag::Has32(int bits, int flag)
{
    return ((bits & (1 << flag)) != 0);
}
