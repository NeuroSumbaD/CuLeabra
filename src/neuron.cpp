#include "neuron.hpp"
#include "bitflag.hpp"

bool leabra::Neuron::HasFlag(NeurFlags flag) {
    return bitflag::Has32(this->Flags, flag);
}

void leabra::Neuron::SetFlag(bool on, std::vector<int> flags) {
    bitflag::Set32((int*)(&(this->Flags)), on, flags);
}

// void leabra::Neuron::SetFlag(NeurFlags flag) {
//     bitflag::Set32((int*)(&(this->Flags)), flag);
// }

// void leabra::Neuron::ClearFlag(NeurFlags flag) {
//     bitflag::Clear32((int*)(&(this->Flags)), flag);
// }

// void leabra::Neuron::SetMask(int mask) {
//     bitflag::Set32((int*)(&(this->Flags)), mask);
// }

// void leabra::Neuron::ClearMask(int mask) {
//     bitflag::Clear32((int*)(&(this->Flags)), mask);
// }

bool leabra::Neuron::IsOff() {
    return HasFlag(NeurOff);
}

std::string leabra::Neuron::StyleType() {
    return "Neuron";
}

std::string leabra::Neuron::StyleClass() {
    return "";
}

std::string leabra::Neuron::StyleName() {
    return "";
}

void leabra::Neuron::InitParamMaps() {
    ParamNameMap["Act"] = (void*) &Act;
    ParamNameMap["ActLrn"] = (void*) &ActLrn;
    ParamNameMap["Ge"] = (void*) &Ge;
    ParamNameMap["Gi"] = (void*) &Gi;
    ParamNameMap["Gk"] = (void*) &Gk;
    ParamNameMap["Inet"] = (void*) &Inet;
    ParamNameMap["Vm"] = (void*) &Vm;
    ParamNameMap["Targ"] = (void*) &Targ;
    ParamNameMap["Ext"] = (void*) &Ext;
    ParamNameMap["AvgSS"] = (void*) &AvgSS;
    ParamNameMap["AvgS"] = (void*) &AvgS;
    ParamNameMap["AvgM"] = (void*) &AvgM;
    ParamNameMap["AvgL"] = (void*) &AvgL;
    ParamNameMap["AvgLLrn"] = (void*) &AvgLLrn;
    ParamNameMap["AvgSLrn"] = (void*) &AvgSLrn;
    ParamNameMap["ActQ0"] = (void*) &ActQ0;
    ParamNameMap["ActQ1"] = (void*) &ActQ1;
    ParamNameMap["ActQ2"] = (void*) &ActQ2;
    ParamNameMap["ActQM"] = (void*) &ActQM;
    ParamNameMap["ActM"] = (void*) &ActM;
    ParamNameMap["ActP"] = (void*) &ActP;
    ParamNameMap["ActDif"] = (void*) &ActDif;
    ParamNameMap["ActDel"] = (void*) &ActDel;
    ParamNameMap["ActAvg"] = (void*) &ActAvg;
    ParamNameMap["Noise"] = (void*) &Noise;
    ParamNameMap["GiSyn"] = (void*) &GiSyn;
    ParamNameMap["GiSelf"] = (void*) &GiSelf;
    ParamNameMap["ActSent"] = (void*) &ActSent;
    ParamNameMap["GeRaw"] = (void*) &GeRaw;
    ParamNameMap["GiRaw"] = (void*) &GiRaw;
    ParamNameMap["GknaFast"] = (void*) &GknaFast;
    ParamNameMap["GknaMed"] = (void*) &GknaMed;
    ParamNameMap["GknaSlow"] = (void*) &GknaSlow;
    ParamNameMap["Spike"] = (void*) &Spike;
    ParamNameMap["ISI"] = (void*) &ISI;
    ParamNameMap["ISIAvg"] = (void*) &ISIAvg;

    ParamTypeMap["Act"] = &typeid(float);
    ParamTypeMap["ActLrn"] = &typeid(float);
    ParamTypeMap["Ge"] = &typeid(float);
    ParamTypeMap["Gi"] = &typeid(float);
    ParamTypeMap["Gk"] = &typeid(float);
    ParamTypeMap["Inet"] = &typeid(float);
    ParamTypeMap["Vm"] = &typeid(float);
    ParamTypeMap["Targ"] = &typeid(float);
    ParamTypeMap["Ext"] = &typeid(float);
    ParamTypeMap["AvgSS"] = &typeid(float);
    ParamTypeMap["AvgS"] = &typeid(float);
    ParamTypeMap["AvgM"] = &typeid(float);
    ParamTypeMap["AvgL"] = &typeid(float);
    ParamTypeMap["AvgLLrn"] = &typeid(float);
    ParamTypeMap["AvgSLrn"] = &typeid(float);
    ParamTypeMap["ActQ0"] = &typeid(float);
    ParamTypeMap["ActQ1"] = &typeid(float);
    ParamTypeMap["ActQ2"] = &typeid(float);
    ParamTypeMap["ActQM"] = &typeid(float);
    ParamTypeMap["ActM"] = &typeid(float);
    ParamTypeMap["ActP"] = &typeid(float);
    ParamTypeMap["ActDif"] = &typeid(float);
    ParamTypeMap["ActDel"] = &typeid(float);
    ParamTypeMap["ActAvg"] = &typeid(float);
    ParamTypeMap["Noise"] = &typeid(float);
    ParamTypeMap["GiSyn"] = &typeid(float);
    ParamTypeMap["GiSelf"] = &typeid(float);
    ParamTypeMap["ActSent"] = &typeid(float);
    ParamTypeMap["GeRaw"] = &typeid(float);
    ParamTypeMap["GiRaw"] = &typeid(float);
    ParamTypeMap["GknaFast"] = &typeid(float);
    ParamTypeMap["GknaMed"] = &typeid(float);
    ParamTypeMap["GknaSlow"] = &typeid(float);
    ParamTypeMap["Spike"] = &typeid(float);
    ParamTypeMap["ISI"] = &typeid(float);
    ParamTypeMap["ISIAvg"] = &typeid(float);
}
