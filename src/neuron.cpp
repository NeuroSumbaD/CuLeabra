#include "neuron.hpp"
#include "bitflag.hpp"

bool leabra::Neuron::HasFlag(NeurFlags flag)
{
    return bitflag::Has32(this->Flags, flag);
}

void leabra::Neuron::SetFlag(NeurFlags flag) {
    bitflag::Set32((int*)(&(this->Flags)), flag);
}

void leabra::Neuron::ClearFlag(NeurFlags flag) {
    bitflag::Clear32((int*)(&(this->Flags)), flag);
}

void leabra::Neuron::SetMask(int mask) {
    bitflag::Set32((int*)(&(this->Flags)), mask);
}

void leabra::Neuron::ClearMask(int mask) {
    bitflag::Clear32((int*)(&(this->Flags)), mask);
}

bool leabra::Neuron::IsOff() {
    return HasFlag(NeurOff);
}