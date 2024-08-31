#include "layer.hpp"

void leabra::Layer::Defaults() {
    Act.Defaults();
    Inhib.Defaults();
    Learn.Defaults();
    Inhib.Layer.On = true;
    for (Path &pt: RecvPaths) {
        pt.Defaults();
    }
}

void leabra::Layer::UpdateParams() {
    Act.Update();
    Inhib.Update();
    Learn.Update();
    Inhib.Layer.On = true;
    for (Path &pt: RecvPaths) {
        pt.UpdateParams();
    }
}

// InitWeights initializes the weight values in the network,
// i.e., resetting learning Also calls InitActs.
void leabra::Layer::InitWeights() {
	UpdateParams();
	for (Path &pt: SendPaths) {
		if (pt.Off) {
			
		}
	}

}
