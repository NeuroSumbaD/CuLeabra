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

bool bitflag::Has32(int bits, int flag)
{
    return ((bits & (1 << flag)) != 0);
}
