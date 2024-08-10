#include <iostream>
#include <bitset>
#include "bitflag.h"
#include "neuron.h"

int main(){
    std::cout << "Mask is: " << std::bitset<8>(bitflag::Mask32(1, 2, 4, 5)) << std::endl;
    std::cout << "Expected a mask of: 00110110" << std::endl;

    int mask = 0;
    neuron::NeurFlags flag1 = neuron::NeurHasTarg;
    bitflag::Set32(&mask, flag1);
    std::cout << "Second mask has been set to: "  << std::bitset<8>(mask) << "(" << flag1 << ")" << std::endl;

    if (bitflag::Has32(mask, flag1)){
        std::cout << "Mask has been properly set." << std::endl;
    }

    bitflag::Clear32(&mask, flag1);
    if (!bitflag::Has32(mask, flag1)) {
        std::cout << "Mask has been properly cleared: " <<  std::bitset<8>(mask) << std::endl;
    }

    return 0;
}