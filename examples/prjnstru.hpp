#pragma once
#include <iostream>
#include "emer.hpp"
#include "leabra.hpp"
#include "layer.hpp"
#include "minmax.hpp"

namespace leabra {

    // PrjnStru contains the basic structural information for specifying a projection of synaptic
    // connections between two layers, and maintaining all the synaptic connection-level data.
    // The exact same struct object is added to the Recv and Send layers, and it manages everything
    // about the connectivity, and methods on the Prjn handle all the relevant computation.
    // struct PrjnStru: leabra::LeabraPrjn {
    //     bool Off;
    //     std::string Cls;
    //     std::string Notes;

    //     // TODO: Figure out the best way to capture the inheritance of the Layer interface
    //     Layer* Send; // sending layer for this projection
    //     Layer* Recv; // receiving layer for this projection -- the emer.Layer interface can be converted to the specific Layer type you are using, e.g., rlay := prjn.Recv.(*leabra.Layer)
    //     // TODO: Figure out the "Pattern" Interface
    //     emer::Pattern* Pat; // pattern of connectivity
    //     // TODO: Figure out PrjnType interface
    //     emer::PrjnType* Typ; // type of projection -- Forward, Back, Lateral, or extended type in specialized algorithms -- matches against .Cls parameter styles (e.g., .Back etc)

    //     // Note to self: Can use an initializer list when assigning: new int[10] { 1,2,3,4,5,6,7,8,9,10 };
    //     std::vector<int> RConN; // number of recv connections for each neuron in the receiving layer, as a flat list

    //     minmax::AvgMax32 RconNAvgMax; // average and maximum number of recv connections in the receiving layer

    //     std::vector<int> RConIdxSt, // starting index into ConIdx list for each neuron in receiving layer -- just a list incremented by ConN
    //                      RConIdx, // index of other neuron on sending side of projection, ordered by the receiving layer's order of units as the outer loop (each start is in ConIdxSt), and then by the sending layer's units within that
    //                      RSynIdx, // index of synaptic state values for each recv unit x connection, for the receiver projection which does not own the synapses, and instead indexes into sender-ordered list
    //                      SConN; // number of sending connections for each neuron in the sending layer, as a flat list

    //     minmax::AvgMax32 SConNAvgMax; // average and maximum number of sending connections in the sending layer

    //     std::vector<int> SConIdxSt, // starting index into ConIdx list for each neuron in sending layer -- just a list incremented by ConN
    //                      SConIdx; // index of other neuron on receiving side of projection, ordered by the sending layer's order of units as the outer loop (each start is in ConIdxSt), and then by the sending layer's units within that

    //     std::string TypeName(){return "Prjn";};
    //     std::string Class(){return ((leabra::LeabraPrjn*)this)->PrjnTypeName() + " " + Cls;};
    //     std::string Name(){return Send->Nm + " to " + Recv->Nm;};

    //     bool IsOff(){return Off || Recv->IsOff() || Send->IsOff();};
    //     void SetOff(bool off){Off = off;};
    //     void Connect(Layer& slay, Layer& rlay, emer::Pattern& pat, emer::PrjnType& typ);

    //     void Validate(bool logmsg);
    //     void BuildStru();
    //     int SetNIdxSt(std::vector<int> &n, minmax::AvgMax32 avgmax, std::vector<int> idxst, tensor::Int32 tn);
    //     std::string String();
    //     bool ApplyParams(params::Sheet& pars, bool setMsg);
        // std::string NonDefaultParams();
    // };
    
} // namespace prjnstru
