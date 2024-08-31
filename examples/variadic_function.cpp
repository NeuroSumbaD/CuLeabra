#include <iostream>
#include <bitset>
#include "makemask.h"


int main() {
    std::cout << std::bitset<8>(makeMask(1, 2, 4, 5)) << std::endl;
    return 0;
}
