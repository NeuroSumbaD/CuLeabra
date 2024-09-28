#include "culeabra.hpp"

// Below handles the definition of the _culeabra python module and any of its
// submodules. Each c++ object file defines a function for exposing its
// underlying structs and enums to the python module.
PYBIND11_MODULE(_culeabra, m) {

    // Leabra module definitions
    pybind_LeabraNet(m);
    pybind_LeabraLayerTypes(m);
    pybind_LeabraPathTypes(m);    
    
    // Patterns of connections
    pybind11::module_ patterns = m.def_submodule("patterns", "Patterns submodule defines different "
            "patterns of connection between layers. Note: ONLY the Full pattern has been tested so far, "
            "so the other pattern types may fail to initialize properly.");

    pybind_Patterns(patterns);
}
