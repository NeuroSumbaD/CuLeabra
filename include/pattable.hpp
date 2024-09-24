/* 
    PatTable replaces the leabra system for creating input and output tables.
    Will provide functions for reading in CSV and TSV files in the Leabra format,
    but I will also make more simplified formats that are easier to read and edit
    outside of the leabra system.

    In the future, I will also try to extend this interface to use Python Generators
    to stream events from some external library to the leabra network.
*/
#include <map>
#include <vector>
#include <string>
#include "tensor.hpp"

namespace pattable {

    struct ColumnInfo {
        std::vector<std::string> LayerNames;
        std::vector<std::vector<int>> LayerIndices;
        std::map<std::string, std::vector<int>> LayerShapes;
    };

    // An event is a set of patterns that are imposed on layers for an
    // AlphaCycle (100 Cycles of ms time steps). The Event struct maps
    // layer names to tensor patterns.
    typedef std::map<std::string, tensor::Tensor<float>*> Event; 

    // Maps event names to the patterns they describe
    typedef std::map<std::string, Event> Events;

    struct Table {
        std::map<std::string, std::string> MetaData;
        Events events;
        ColumnInfo info;

        std::vector<std::string> eventNames;
        std::vector<int> permutation;
        uint EventIndex = 0;
        uint numLayers = 0;

        Table():MetaData(),events(), permutation(){};
        Table(std::string fileName);

        // void AddToTable(std::string layerName, std::vector<int> shape, std::vector<std::string> names);

        void ReadFile(std::string fileName);

        tensor::Tensor<float> *GetPattern(std::string name);
    };
    
} // namespace pattable
