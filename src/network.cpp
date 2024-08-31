#include "network.hpp"

void leabra::Network::Defaults() {
    WtBalInterval=20;
    WtBalCtr=0;
    int index = 0;
    for (Layer ly: Layers){
        ly.Defaults();
        ly.Index = index++;
    }
}