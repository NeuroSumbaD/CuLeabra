#include "makemask.hpp"

int makeMask(int value) {
    return (1 << value);
}

template <typename... Args>
int makeMask(int first, Args... rest) {
    return (1 << first) + makeMask(rest...);
}