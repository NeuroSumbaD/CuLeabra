#pragma once
#include <string>
#include <map>
#include <vector>
#include <any>

#include "params.hpp"
#include "math.hpp"
#include "rand.hpp"
#include "tensor.hpp"
#include "relpos.hpp"
#include "path.hpp"

// TODO CHECK IF ANY OF THESE TYPES NEED CONSTRUCTORS TO INTIIALIZE DATA

namespace emer {
    struct Network;
    struct Layer;
    struct Path;


    // NetworkBase defines the basic data for a neural network,
    // used for managing the structural elements of a network,
    // and for visualization, I/O, etc.
    struct Network{
        // overall name of network, which helps discriminate if there are multiple.
        std::string Name;

        // filename of last weights file loaded or saved.
        std::string WeightsFile;

        // map of name to layers, for EmerLayerByName methods
        std::map<std::string, Layer> LayerNameMap;

        // map from class name to layer names.
        std::map<std::string, std::vector<std::string>> LayerClassMap;
        
        // minimum display position in network
        math::Vector3 MinPos;

        // maximum display position in network
        math::Vector3 MaxPos;

        // optional metadata that is saved in network weights files,
        // e.g., can indicate number of epochs that were trained,
        // or any other information about this network that would be useful to save.
        std::map<std::string, std::string> MetaData;

        // random number generator for the network.
        // all random calls must use this.
        // Set seed here for weight initialization values.
        rands::SysRand Rand;

        // Random seed to be set at the start of configuring
        // the network and initializing the weights.
        // Set this to get a different set of weights.
        int RandSeed;

        Network(std::string name, std::string weightsFile = "", int randSeed = 0):
            Name(name), WeightsFile(weightsFile), LayerNameMap(), LayerClassMap(), MinPos(), MaxPos(), MetaData(), Rand(), RandSeed(randSeed){};

        void UpdateLayerMaps();
        Layer& LayerByName(std::string name);
        Path& PathByName(std::string name);
        std::vector<Layer&> LayersByClass(std::string className);
        std::vector<Layer&> LayersByClass(std::string classes, ...);
        // void LayoutLayers();
        // void LayoutBoundsUpdate();
        // void VerticalLayerLayout();
        // VarRange(varNm string) (min, max float32, err error)
        
        //Params
        void ApplyParams(params::Sheet& pars, bool setMsg);
        // std::string NonDefaultParams();
        // void SaveAllParams(fstream file);
        // void SaveNonDefaultParams(fstream file);
        void SetRandSeed(int seed){RandSeed = seed; ResetRandSeed();};
        void ResetRandSeed(){Rand.NewSeed(RandSeed);};


        // NETWORK INTERFACE
        // Network defines the minimal interface for a neural network,
        // used for managing the structural elements of a network,
        // and for visualization, I/O, etc.
        // Most of the standard expected functionality is defined in the
        // NetworkBase struct, and this interface only has methods that must be
        // implemented specifically for a given algorithmic implementation.

        // NumLayers returns the number of layers in the network.
        virtual int NumLayers();

        // EmerLayer returns layer as emer.Layer interface at given index.
        // Does not do extra bounds checking.
        virtual Layer EmerLayer(int idx);

        // MaxParallelData returns the maximum number of data inputs that can be
        // processed in parallel by the network.
        // The NetView supports display of up to this many data elements.
        virtual int MaxParallelData();

        // NParallelData returns the current number of data inputs currently being
        // processed in parallel by the network.
        // Logging supports recording each of these where appropriate.
        virtual int NParallelData();

        // Defaults sets default parameter values for everything in the Network.
        virtual void Defaults();

        // UpdateParams() updates parameter values for all Network parameters,
        // based on any other params that might have changed.
        virtual void UpdateParams();

        // TODO: Determine if these methods are useful to implement

        // KeyLayerParams returns a listing for all layers in the network,
        // of the most important layer-level params (specific to each algorithm).
        // virtual std::string KeyLayerParams();

        // KeyPathParams returns a listing for all Recv pathways in the network,
        // of the most important pathway-level params (specific to each algorithm).
        // virtual std::string KeyPathParams();

        // UnitVarNames returns a list of variable names available on
        // the units in this network.
        // This list determines what is shown in the NetView
        // (and the order of vars list).
        // Not all layers need to support all variables,
        // but must safely return math32.NaN() for unsupported ones.
        // This is typically a global list so do not modify!
        // virtual std::vector<std::string> UnitVarNames();

        // UnitVarProps returns a map of unit variable properties,
        // with the key being the name of the variable,
        // and the value gives a space-separated list of
        // go-tag-style properties for that variable.
        // The NetView recognizes the following properties:
        // range:"##" = +- range around 0 for default display scaling
        // min:"##" max:"##" = min, max display range
        // auto-scale:"+" or "-" = use automatic scaling instead of fixed range or not.
        // zeroctr:"+" or "-" = control whether zero-centering is used
        // desc:"txt" tooltip description of the variable
        // Note: this is typically a global list so do not modify!
        // virtual std::map<std::string, std::string> UnitVarProps();

        // SynVarNames returns the names of all the variables
        // on the synapses in this network.
        // This list determines what is shown in the NetView
        // (and the order of vars list).
        // Not all pathways need to support all variables,
        // but must safely return math32.NaN() for
        // unsupported ones.
        // This is typically a global list so do not modify!
        // virtual std::vector<std::string> SynVarNames();

        // SynVarProps returns a map of synapse variable properties,
        // with the key being the name of the variable,
        // and the value gives a space-separated list of
        // go-tag-style properties for that variable.
        // The NetView recognizes the following properties:
        // range:"##" = +- range around 0 for default display scaling
        // min:"##" max:"##" = min, max display range
        // auto-scale:"+" or "-" = use automatic scaling instead of fixed range or not.
        // zeroctr:"+" or "-" = control whether zero-centering is used
        // Note: this is typically a global list so do not modify!
        // virtual std::map<std::string, std::string> SynVarProps();
    };

    // LayerBase defines the basic shared data for neural network layers,
    // used for managing the structural elements of a network,
    // and for visualization, I/O, etc.
    // Nothing algorithm-specific is implemented here
    struct Layer: params::StylerObject{
        // Name of the layer, which must be unique within the network.
        // Layers are typically accessed directly by name, via a map.
        std::string Name;

        // Class is for applying parameter styles across multiple layers
        // that all get the same parameters.  This can be space separated
        // with multple classes.
        std::string Class; // string

        // Info contains descriptive information about the layer.
        // This is displayed in a tooltip in the network view.
        std::string Info; // string

        // Off turns off the layer, removing from all computations.
        // This provides a convenient way to dynamically test for
        // the contributions of the layer, for example.
        bool Off; // bool

        // Shape of the layer, either 2D or 4D.  Although spatial topology
        // is not relevant to all algorithms, the 2D shape is important for
        // efficiently visualizing large numbers of units / neurons.
        // 4D layers have 2D Pools of units embedded within a larger 2D
        // organization of such pools.  This is used for max-pooling or
        // pooled inhibition at a finer-grained level, and biologically
        // corresopnds to hypercolumns in the cortex for example.
        // Order is outer-to-inner (row major), so Y then X for 2D;
        // 4D: Y-X unit pools then Y-X neurons within pools.
        tensor::Shape Shape; // tensor.Shape

        // Pos specifies the relative spatial relationship to another
        // layer, which determines positioning.  Every layer except one
        // "anchor" layer should be positioned relative to another,
        // e.g., RightOf, Above, etc.  This provides robust positioning
        // in the face of layer size changes etc.
        // Layers are arranged in X-Y planes, stacked vertically along the Z axis.
        relpos::Pos Pos; // relpos.Pos `table:"-" display:"inline"`

        // Index is a 0..n-1 index of the position of the layer within
        // the list of layers in the network.
        int Index; // int `display:"-" edit:"-"`

        // SampleIndexes are the current set of "sample" unit indexes,
        // which are a smaller subset of units that represent the behavior
        // of the layer, for computationally intensive statistics and displays
        // (e.g., PCA, ActRF, NetView rasters), when the layer is large.
        // If none have been set, then all units are used.
        // See utility function CenterPoolIndexes that returns indexes of
        // units in the central pools of a 4D layer.
        std::vector<int> SampleIndexes; // []int `table:"-"`

        // SampleShape is the shape to use for the subset of sample
        // unit indexes, in terms of an array of dimensions.
        // See Shape for more info.
        // Layers that set SampleIndexes should also set this,
        // otherwise a 1D array of len SampleIndexes will be used.
        // See utility function CenterPoolShape that returns shape of
        // units in the central pools of a 4D layer.
        tensor::Shape SampleShape; // tensor.Shape `table:"-"`

        // provides a history of parameters applied to the layer
        // ParamsHistory; // params.HistoryImpl `table:"-"`

        // optional metadata that is saved in network weights files,
        // e.g., can indicate number of epochs that were trained,
        // or any other information about this network that would be useful to save.
        std::map<std::string,std::string> MetaData; // map[string]string

        Layer(std::string name, int index = 0, std::vector<int> shape = {1,1}):Name(name), Off(true), Shape(shape), Pos(), Index(0), SampleIndexes(), SampleShape(shape), MetaData(){};
        // TODO check if initialization of shape is OK

        std::string StyleType(){return "Layer";};
        std::string StyleClass(){return this->TypeName() + " " + this->Class;};
        std::string StyleName(){return this->Name;};

        void AddClass(std::string cls);
        void AddClass(std::string cls, ...);

        std::string Label(){return Name;};
        bool Is2D(){return Shape.NumDims() == 2;};
        bool Is4D(){return Shape.NumDims() == 4;};
        int NumUnits(){return Shape.Len();};
        std::tuple<std::vector<int>, bool> Index4DFrom2D(int x, int y);
        void PlaceRightOf(Layer &other, float space);
        void PlaceBehind(Layer &other, float space);
        void PlaceAbove(Layer &other, float space);
        math::Vector2 DisplaySize();
        void SetShape(std::vector<int> shape);
        void SetSampleIndexesShape(std::vector<int> idxs, std::vector<int> shape);
        tensor::Shape GetSampleShape();
        int NumPools();
        // void UnitValues(std::vector<float> &vals, std::string VarNm, int di);
        // void UnitValuesTensor(tensor::Tensor &vals, std::string VarNm, int di);
        // void UnitValuesSampleTensor(tensor::Tensor &vals, std::string VarNm, int di);
        // void UnitValue(std::string VarNm, std::vector<int> idx, int di);
        // std::vector<int> CenterPoolIndexes(Layer &ly, int n);
        // std::vector<int> CenterPoolShape(Layer &ly, int n);
        std::tuple<std::vector<int>,std::vector<int>> Layer2DSampleIndexes(Layer &ly, int maxSize);
        Path& RecvPathBySendName(std::string sender);
        Path& SendPathByRecvName(std::string sender);
        Path& RecvPathBySendNameType(std::string sender, std::string typeName);
        Path& SendPathByRecvNameType(std::string sender, std::string typeName);
        //Params
        void SetParam(params::Sel &sel);
        bool ApplyParams(params::Sheet &pars, bool setMsg);

        // LAYER INTERFACE
        // Layer defines the minimal interface for neural network layers,
        // necessary to support the visualization (NetView), I/O,
        // and parameter setting functionality provided by emergent.
        // Most of the standard expected functionality is defined in the
        // LayerBase struct, and this interface only has methods that must be
        // implemented specifically for a given algorithmic implementation.
        
        // TypeName is the type or category of layer, defined
        // by the algorithm (and usually set by an enum).
        virtual std::string TypeName();

        // UnitVarIndex returns the index of given variable within
        // the Neuron, according to *this layer's* UnitVarNames() list
        // (using a map to lookup index), or -1 and error message if
        // not found.
        virtual int UnitVarIndex(std::string varNm);// (int, error)

        // UnitValue1D returns value of given variable index on given unit,
        // using 1-dimensional index, and a data parallel index di,
        // for networks capable of processing multiple input patterns
        // in parallel. Returns NaN on invalid index.
        // This is the core unit var access method used by other methods,
        // so it is the only one that needs to be updated for derived layer types.
        virtual float UnitValue1D(int varIndex, int idx, int di);

        // VarRange returns the min / max values for given variable
        virtual std::tuple<float,float> VarRange(std::string varNm);

        // NumRecvPaths returns the number of receiving pathways.
        virtual int NumRecvPaths();

        // RecvPath returns a specific receiving pathway.
        virtual Path& RecvPath(int idx);

        // NumSendPaths returns the number of sending pathways.
        virtual int NumSendPaths();

        // SendPath returns a specific sending pathway.
        virtual Path& SendPath(int idx);

        // RecvPathValues fills in values of given synapse variable name,
        // for pathway from given sending layer and neuron 1D index,
        // for all receiving neurons in this layer,
        // into given float32 slice (only resized if not big enough).
        // pathType is the string representation of the path type;
        // used if non-empty, useful when there are multiple pathways
        // between two layers.
        // Returns error on invalid var name.
        // If the receiving neuron is not connected to the given sending
        // layer or neuron then the value is set to math32.NaN().
        // Returns error on invalid var name or lack of recv path
        // (vals always set to nan on path err).
        virtual void RecvPathValues(std::vector<float> &vals, std::string varNm, Layer &sendLay, int sendIndex1D, std::string pathType);

        // SendPathValues fills in values of given synapse variable name,
        // for pathway into given receiving layer and neuron 1D index,
        // for all sending neurons in this layer,
        // into given float32 slice (only resized if not big enough).
        // pathType is the string representation of the path type -- used if non-empty,
        // useful when there are multiple pathways between two layers.
        // Returns error on invalid var name.
        // If the sending neuron is not connected to the given receiving layer or neuron
        // then the value is set to math32.NaN().
        // Returns error on invalid var name or lack of recv path (vals always set to nan on path err).
        virtual void SendPathValues(std::vector<float> &vals, std::string varNm, Layer &recvLay, int recvIndex1D, std::string pathType);

        // UpdateParams() updates parameter values for all Layer
        // and recv pathway parameters,
        // based on any other params that might have changed.
        virtual void UpdateParams();

        // SetParam sets parameter at given path to given value.
        // returns error if path not found or value cannot be set.
        virtual void SetParam(std::string path, std::string val);

        // NonDefaultParams returns a listing of all parameters in the Layer that
        // are not at their default values -- useful for setting param styles etc.
        virtual std::string NonDefaultParams();

        // AllParams returns a listing of all parameters in the Layer
        virtual std::string AllParams();

        // WriteWeightsJSON writes the weights from this layer from the
        // receiver-side perspective in a JSON text format.
        // void WriteWeightsJSON(std::ifstream jsonFile, int depth);

        // SetWeights sets the weights for this layer from weights.Layer
        // decoded values
        // void SetWeights(weights::Layer lw);
    };

    // PathBase defines the basic shared data for a pathway
    // which connects two layers, using a specific Pattern
    // of connectivity, and with its own set of parameters.
    // The same struct token is added to the Recv and Send
    // layer path lists,
    struct Path: params::StylerObject{
        // Name of the path, which can be automatically set to
        // SendLayer().Name + "To" + RecvLayer().Name via
        // SetStandardName method.
        std::string Name;

        // Class is for applying parameter styles across multiple paths
        // that all get the same parameters.  This can be space separated
        // with multple classes.
        std::string Class;

        // Info contains descriptive information about the pathway.
        // This is displayed in a tooltip in the network view.
        std::string Info;

        // can record notes about this pathway here.
        std::string Notes;

        // Pattern specifies the pattern of connectivity
        // for interconnecting the sending and receiving layers.
        paths::Pattern *Pattern;

        // Off inactivates this pathway, allowing for easy experimentation.
        bool Off;

        Path(std::string name = "", std::string cls=""):Name(name), Class(cls), Info(), Notes(){Pattern = nullptr;};

        // provides a history of parameters applied to the layer
        // ParamsHistory params.HistoryImpl `table:"-"

        std::string StyleType(){return "Path";};
        std::string StyleClass(){return TypeName() + " " + Class;};
        std::string StyleName(){return Name;};
        std::string Label(){return Name;};

        // std::string AddClass(std::string cls);
        // template <typename... Args>
        // void AddClass(std::string cls, Args... rest);
        void AddClass(std::vector<std::string> classes);

        // float SynValue(std::string varNm, int sidx, int ridx);
        // void ParamsHistorReset();
        // void ParamsApplied(params::Sel &sel);
        void SetParam(std::string path, std::string val);
        bool ApplyParams(params::Sheet &pars, bool setMsg);
        // std::string NonDefaultParams();

        // PATH INTERFACE
        // Path defines the minimal interface for a pathway
        // which connects two layers, using a specific Pattern
        // of connectivity, and with its own set of parameters.
        // This supports visualization (NetView), I/O,
        // and parameter setting functionality provided by emergent.
        // Most of the standard expected functionality is defined in the
        // PathBase struct, and this interface only has methods that must be
        // implemented specifically for a given algorithmic implementation,

        // TypeName is the type or category of path, defined
        // by the algorithm (and usually set by an enum).
        std::string TypeName();

        // SendLayer returns the sending layer for this pathway,
        // as an emer.Layer interface.  The actual Path implmenetation
        // can use a Send field with the actual Layer struct type.
        // virtual Layer& SendLayer();

        // RecvLayer returns the receiving layer for this pathway,
        // as an emer.Layer interface.  The actual Path implmenetation
        // can use a Recv field with the actual Layer struct type.
        // virtual Layer& RecvLayer();

        // NumSyns returns the number of synapses for this path.
        // This is the max idx for SynValue1D and the number
        // of vals set by SynValues.
        virtual int NumSyns();

        // SynIndex returns the index of the synapse between given send, recv unit indexes
        // (1D, flat indexes). Returns -1 if synapse not found between these two neurons.
        // This requires searching within connections for receiving unit (a bit slow).
        virtual int SynIndex(int sidx, int ridx);

        // SynVarNames returns the names of all the variables on the synapse
        // This is typically a global list so do not modify!
        virtual std::vector<std::string> SynVarNames();

        // SynVarNum returns the number of synapse-level variables
        // for this paths.  This is needed for extending indexes in derived types.
        virtual int SynVarNum();

        // SynVarIndex returns the index of given variable within the synapse,
        // according to *this path's* SynVarNames() list (using a map to lookup index),
        // or -1 and error message if not found.
        virtual int SynVarIndex(std::string varNm);

        // SynValues sets values of given variable name for each synapse,
        // using the natural ordering of the synapses (sender based for Axon),
        // into given float32 slice (only resized if not big enough).
        // Returns error on invalid var name.
        virtual void SynValues(std::vector<float> &vals, std::string varNm);

        // SynValue1D returns value of given variable index
        // (from SynVarIndex) on given SynIndex.
        // Returns NaN on invalid index.
        // This is the core synapse var access method used by other methods,
        // so it is the only one that needs to be updated for derived types.
        virtual float SynValue1D(int varIndex, int synIndex);

        // UpdateParams() updates parameter values for all Path parameters,
        // based on any other params that might have changed.
        virtual void UpdateParams();

        // SetParam sets parameter at given path to given value.
        // returns error if path not found or value cannot be set.
        virtual void SetParam(std::string path, std::string val);

        // AllParams returns a listing of all parameters in the Pathway.
        virtual std::string AllParams();

        // WriteWeightsJSON writes the weights from this pathway
        // from the receiver-side perspective in a JSON text format.
        // virtual void WriteWeightsJSON(std::ifstream jsonFile, int depth);

        // SetWeights sets the weights for this pathway from weights.Path
        // decoded values
        // virtual void SetWeights(weights::Layer lw);
    };

} // namespace emer