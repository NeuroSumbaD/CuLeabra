#pragma once
#include <vector>

namespace bitflag{
    void SetMask32(int* bits, int mask);

    int Mask32(int flag);
    template <typename... Args> int Mask32(int flag, Args... args);

    template <typename... Args> void Set32(int* bits, Args... flags);

    void ClearMask32(int* bits, int mask);
    template <typename... Args> void Clear32(int* bits, Args... flags);

    template <typename... Args> void Set32(int* bits, bool on, Args... flags);
    
    void Set32(int* bits, bool on, std::vector<int> flags);

    bool Has32(int bits, int flag);

}

// NOTE TO SELF: Template definitions need to go in headers...

// Recursive variadic template function
// Accepts any number of int inputs representing bit indices and creates a bitmask
template <typename... Args>
int bitflag::Mask32(int first, Args... rest) {
    return (1 << first) + bitflag::Mask32(rest...);
}

template <typename... Args>
void bitflag::Set32(int *bits, Args... flags) {
    SetMask32(bits, Mask32(flags...));
}

template <typename... Args>
void bitflag::Clear32(int *bits, Args... flags) {
    ClearMask32(bits, Mask32(flags...));
}
