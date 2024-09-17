#include "pool.hpp"

leabra::Pool::Pool():Inhib(), ActM(), ActP(), ActAvgs() {
    StIndex = 0;
    EdIndex = 0;
}

void leabra::Pool::Init(){
    Inhib.Init();
}
