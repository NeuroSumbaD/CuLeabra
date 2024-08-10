#include <iostream>
#include <bitset>


int makeMask(int value) {
    return (1 << value);
}

template <typename... Args>
int makeMask(int first, Args... rest) {
    return (1 << first) + makeMask(rest...);
}

int main() {
    std::cout << std::bitset<8>(makeMask(1, 2, 4, 5)) << std::endl;
    return 0;
}
