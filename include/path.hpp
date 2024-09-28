#pragma once
#include <fstream>
#include <functional>
#include <pybind11/pybind11.h>

#include "learn.hpp"
#include "act.hpp"
#include "synapse.hpp"
#include "tensor.hpp"
#include "math.hpp"


namespace paths {
    // GaussTopo has parameters for Gaussian topographic weights or scaling factors
    struct GaussTopo {
        // use gaussian topographic weights / scaling values
        bool On;

        // gaussian sigma (width) in normalized units where entire distance across relevant dimension is 1.0 -- typical useful values range from .3 to 1.5, with .6 default
        float Sigma = 0.6;

        // wrap the gaussian around on other sides of the receptive field, with the closest distance being used -- this removes strict topography but ensures a more uniform distribution of weight values so edge units don't have weaker overall weights
        bool Wrap = true;

        // proportion to move gaussian center relative to the position of the receiving unit within its pool: 1.0 = centers span the entire range of the receptive field.  Typically want to use 1.0 for Wrap = true, and 0.8 for false
        float CtrMove = 1;

        void Defaults();
    };

    // SigmoidTopo has parameters for Sigmoidal topographic weights or scaling factors
    struct SigmoidTopo {
        // use gaussian topographic weights / scaling values
        bool On;

        // gain of sigmoid that determines steepness of curve, in normalized units where entire distance across relevant dimension is 1.0 -- typical useful values range from 0.01 to 0.1
        float Gain = 0.05;

        // proportion to move gaussian center relative to the position of the receiving unit within its pool: 1.0 = centers span the entire range of the receptive field.  Typically want to use 1.0 for Wrap = true, and 0.8 for false
        float CtrMove = 0.5;

        void Defaults();
    };

    // Pattern defines a pattern of connectivity between two layers.
    // The pattern is stored efficiently using a bitslice tensor of binary values indicating
    // presence or absence of connection between two items.
    // A receiver-based organization is generally assumed but connectivity can go either way.
    struct Pattern {
        std::string type = "Pattern";
        // Name returns the name of the pattern -- i.e., the "type" name of the actual pattern generatop
        virtual std::string Name() = 0;

        // Connect connects layers with the given shapes, returning the pattern of connectivity
        // as a bits tensor with shape = recv + send shapes, using row-major ordering with outer-most
        // indexes first (i.e., for each recv unit, there is a full inner-level of sender bits).
        // The number of connections for each recv and each send unit are also returned in
        // recvn and send tensors, each the shape of send and recv respectively.
        // The same flag should be set to true if the send and recv layers are the same (i.e., a self-connection)
        // often there are some different options for such connections.
        virtual std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> Connect(tensor::Shape &send, tensor::Shape &recv, bool same) = 0;
    };

    std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> NewTensors(tensor::Shape &send, tensor::Shape &recv);
    
    // Full implements full all-to-all pattern of connectivity between two layers
    struct Full: Pattern {
        std::string type = "Full";
        // if true, and connecting layer to itself (self pathway), then make a self-connection from unit to itself
        bool SelfCon;

        std::string Name(){return "Full";};
        std::tuple<tensor::Int32*, tensor::Int32*, tensor::Bits*> Connect(tensor::Shape &send, tensor::Shape &recv, bool same);
    };

    // PoolTile implements tiled 2D connectivity between pools within layers, where
    // a 2D rectangular receptive field (defined over pools, not units) is tiled
    // across the sending layer pools, with specified level of overlap.
    // Pools are the outer-most two dimensions of a 4D layer shape.
    // 2D layers are assumed to have 1x1 pool.
    // This is a standard form of convolutional connectivity, where pools are
    // the filters and the outer dims are locations filtered.
    // Various initial weight / scaling patterns are also available -- code
    // must specifically apply these to the receptive fields.
    // TODO IMPLEMENT POOLTILE
    struct PoolTile: Pattern {
        std::string type = "PoolTile";

        // reciprocal topographic connectivity -- logic runs with recv <-> send -- produces symmetric back-pathway or topo path when sending layer is larger than recv
        bool Recip;

        // size of receptive field tile, in terms of pools on the sending layer
        vecint::Vector2i Size;

        // how many pools to skip in tiling over sending layer -- typically 1/2 of Size
        vecint::Vector2i Skip;

        // starting pool offset for lower-left corner of first receptive field in sending layer
        vecint::Vector2i Start;

        // if true, pool coordinates wrap around sending shape -- otherwise truncated at edges, which can lead to assymmetries in connectivity etc
        bool Wrap;

        // gaussian topographic weights / scaling parameters for full receptive field width. multiplies any other factors present
        GaussTopo GaussFull;

        // gaussian topographic weights / scaling parameters within individual sending pools (i.e., unit positions within their parent pool drive distance for gaussian) -- this helps organize / differentiate units more within pools, not just across entire receptive field. multiplies any other factors present
        GaussTopo GaussInPool;

        // sigmoidal topographic weights / scaling parameters for full receptive field width.  left / bottom half have increasing sigmoids, and second half decrease.  Multiplies any other factors present (only used if Gauss versions are not On!)
        SigmoidTopo SigFull;

        // sigmoidal topographic weights / scaling parameters within individual sending pools (i.e., unit positions within their parent pool drive distance for sigmoid) -- this helps organize / differentiate units more within pools, not just across entire receptive field. multiplies any other factors present  (only used if Gauss versions are not On!).  left / bottom half have increasing sigmoids, and second half decrease.
        SigmoidTopo SigInPool;

        // min..max range of topographic weight values to generate
        minmax::F32 TopoRange;

        bool HasTopoWeights();

        void TopoWeights(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts);

        void TopoWeightsGauss2D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts);
        void TopoWeightsGauss4D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts);

        void TopoWeightsSigmoid2D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts);
        void TopoWeightsSigmoid4D(tensor::Shape &send, tensor::Shape &recv, tensor::Tensor<float> &wts);
    };

    float gaussWts(int si, int ri, tensor::Shape &send, tensor::Shape &recv);

    // Circle implements a circular pattern of connectivity between two layers
    // where the center moves in proportion to receiver position with offset
    // and multiplier factors, and a given radius is used (with wrap-around
    // optionally).  A corresponding Gaussian bump of TopoWeights is available as well.
    // Makes for a good center-surround connectivity pattern.
    // 4D layers are automatically flattened to 2D for this connection.
    struct Circle: Pattern {
        std::string type = "Circle";

        // radius of the circle, in units from center in sending layer
        int Radius;

        // starting offset in sending layer, for computing the corresponding sending center relative to given recv unit position
        vecint::Vector2i Start;

        // scaling to apply to receiving unit position to compute sending center as function of recv unit position
        math::Vector2 Scale;

        // auto-scale sending center positions as function of relative sizes of send and recv layers -- if Start is positive then assumes it is a border, subtracted from sending size
        bool AutoScale;

        // if true, connectivity wraps around edges
        bool Wrap;

        // if true, this path should set gaussian topographic weights, according to following parameters
        bool TopoWeights;

        // gaussian sigma (width) as a proportion of the radius of the circle
        float Sigma;

        // maximum weight value for GaussWts function -- multiplies values
        float MaxWt;

        // if true, and connecting layer to itself (self pathway), then make a self-connection from unit to itself
        bool SelfCon;

        std::function<float (int si, int ri, tensor::Shape &send, tensor::Shape &recv)> GaussWts = gaussWts;
    };
    

} // namespace paths

void pybind_Patterns(pybind11::module_ &m);