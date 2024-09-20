#include "leabra.hpp"
#include "params.hpp"

int main(){
    // TODO: Find a way to clean up this syntax
    // TODO: Find a way to create and read JSON param files
    params::Sets ParamSets = 
        {
            {
                "Base",
                {
                    {
                        Sel: "Path",
                        Desc: "norm and momentum on works better, but wt bal is not better for smaller nets",
                        ParamsSet: {
                            {"Path.Learn.Norm.On",     "true"},
                            {"Path.Learn.Momentum.On", "true"},
                            {"Path.Learn.WtBal.On",    "true"}, // no diff really
                            // "Path.Learn.WtBal.Targs": "true", // no diff here
                        }
                    },
                    {
                        Sel: "Layer",
                        Desc: "using default 1.8 inhib for all of network -- can explore",
                        ParamsSet: {
                            {"Layer.Inhib.Layer.Gi", "1.8"},
                            {"Layer.Act.Init.Decay", "0.0"},
                            {"Layer.Act.Gbar.L",     "0.1"}, // set explictly, new default, a bit better vs 0.2
                        }
                    },
                    {
                        Sel: ".BackPath",
                        Desc: "top-down back-pathways MUST have lower relative weight scale, otherwise network hallucinates",
                        ParamsSet: {
                            {"Path.WtScale.Rel", "0.2"},
                        }
                    },
                    {
                        Sel: "#Output",
                        Desc: "output definitely needs lower inhib -- true for smaller layers in general",
                        ParamsSet: {
                            {"Layer.Inhib.Layer.Gi", "1.4"},
                        }
                    },
                },
            },
            {
                "DefaultInhib", 
                {
                    {
                        Sel: "#Output",
                        Desc: "go back to default",
                        ParamsSet: {
                            {"Layer.Inhib.Layer.Gi", "1.8"},
                        }
                    },
                }
            },
            {
                "NoMomentum",
                {
                    {
                        Sel: "Path",
                        Desc: "no norm or momentum",
                        ParamsSet: {
                            {"Path.Learn.Norm.On",     "false"},
                            {"Path.Learn.Momentum.On", "false"},
                        }
                    },
                }
            },
            {
                "WtBalOn",
                {
                    {
                        Sel: "Path",
                        Desc: "weight bal on",
                        ParamsSet: {
                            {"Path.Learn.WtBal.On", "true"},
                        }
                    },
                }
            },
    };
}