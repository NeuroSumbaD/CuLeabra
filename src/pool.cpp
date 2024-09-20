#include "pool.hpp"

leabra::Pool::Pool():Inhib(), ActM(), ActP(), ActAvgs() {
    StIndex = 0;
    EdIndex = 0;
}

void leabra::Pool::Init(){
    Inhib.Init();
}

std::string leabra::Pool::StyleType() {
    return "Pool";
}

std::string leabra::Pool::StyleClass() {
    return "";
}

std::string leabra::Pool::StyleName() {
    return "";
}

void leabra::Pool::InitParamMaps() {
    ParamNameMap["Inhib"] = (void*) &Inhib;

    ParamTypeMap["Inhib"] = &typeid(Inhib);
}
