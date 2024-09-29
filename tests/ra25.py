import culeabra
from culeabra import params
from culeabra import patterns
from culeabra import environments

import numpy as np
import matplotlib.pyplot as plt

testSel = params.CreateSel({
                    "Sel": "Path",
                    "Desc": "norm and momentum on works better, but wt bal is not better for smaller nets",
                    "ParamsSet": {
                        "Path.Learn.Norm.On":     "true",
                        "Path.Learn.Momentum.On": "true",
                        "Path.Learn.WtBal.On":    "true", # no diff really
                        # "Path.Learn.WtBal.Targs": "true", # no diff here
                    }
                })

testSheet = params.Sheet([
        params.CreateSel({
            "Sel": "Path",
            "Desc": "norm and momentum on works better, but wt bal is not better for smaller nets",
            "ParamsSet": {
                "Path.Learn.Norm.On":     "true",
                "Path.Learn.Momentum.On": "true",
                "Path.Learn.WtBal.On":    "true", # no diff really
                # "Path.Learn.WtBal.Targs": "true", # no diff here
            }
        }),
        params.CreateSel({
            "Sel": "Layer",
            "Desc": "using default 1.8 inhib for all of network -- can explore",
            "ParamsSet": {
                "Layer.Inhib.Layer.Gi": "1.8",
                "Layer.Act.Init.Decay": "0.0",
                "Layer.Act.Gbar.L":     "0.1", # set explictly, new default, a bit better vs 0.2
            }
        }),
        params.CreateSel({
            "Sel": ".BackPath",
            "Desc": "top-down back-pathways MUST have lower relative weight scale, otherwise network hallucinates",
            "ParamsSet": {
                "Path.WtScale.Rel": "0.2",
            }
        }),
        params.CreateSel({
            "Sel": "#Output",
            "Desc": "output definitely needs lower inhib -- true for smaller layers in general",
            "ParamsSet": {
                "Layer.Inhib.Layer.Gi": "1.4",
            }
        }),
    ])

paramSets = params.Sets(
    {
        "Base": [
            params.CreateSel({
                    "Sel": "Path",
                    "Desc": "norm and momentum on works better, but wt bal is not better for smaller nets",
                    "ParamsSet": {
                        "Path.Learn.Norm.On":     "true",
                        "Path.Learn.Momentum.On": "true",
                        "Path.Learn.WtBal.On":    "true", # no diff really
                        # "Path.Learn.WtBal.Targs": "true", # no diff here
                    }
            }),
            params.CreateSel({
                "Sel": "Layer",
                "Desc": "using default 1.8 inhib for all of network -- can explore",
                "ParamsSet": {
                    "Layer.Inhib.Layer.Gi": "1.8",
                    "Layer.Act.Init.Decay": "0.0",
                    "Layer.Act.Gbar.L":     "0.1", # set explictly, new default, a bit better vs 0.2
                }
            }),
            params.CreateSel({
                "Sel": ".BackPath",
                "Desc": "top-down back-pathways MUST have lower relative weight scale, otherwise network hallucinates",
                "ParamsSet": {
                    "Path.WtScale.Rel": "0.2",
                }
            }),
            params.CreateSel({
                "Sel": "#Output",
                "Desc": "output definitely needs lower inhib -- true for smaller layers in general",
                "ParamsSet": {
                    "Layer.Inhib.Layer.Gi": "1.4",
                }
            }),
        ],
        "DefaultInhib": [
            params.CreateSel({
                "Sel": "#Output",
                "Desc": "go back to default",
                "ParamsSet": {
                    "Layer.Inhib.Layer.Gi": "1.8",
                }
            }),
        ],
        "NoMomentum": [
            params.CreateSel({
                "Sel": "Path",
                "Desc": "no norm or momentum",
                "ParamsSet": {
                    "Path.Learn.Norm.On":    "false",
                    "Path.Learn.Momentum.On": "false",
                }
            }),
        ],
        "WtBalOn": [
            params.CreateSel({
                "Sel": "Path",
                "Desc": "weight bal on",
                "ParamsSet": {
                    "Path.Learn.WtBal.On": "true",
                }
            }),
        ]
    })

# create net
net = culeabra.Network("RA25")

# define layers
inp = net.AddLayer2D("Input", 5, 5, culeabra.InputLayer)
hid1 = net.AddLayer2D("Hidden1", 7, 7, culeabra.SuperLayer)
hid2 = net.AddLayer2D("Hidden2", 7, 7, culeabra.SuperLayer)
out = net.AddLayer2D("Output", 5, 5, culeabra.InputLayer)

# define connectivity pattern
full = patterns.Full()

# create connections
net.ConnectLayers(inp, hid1, full, culeabra.ForwardPath)
net.BidirConnectLayers(hid1, hid2, full)
net.BidirConnectLayers(hid2, out, full)

env = environments.TabulatedEnv("random_5x5_25.tsv")

print("Creating simulation object...")
sim = culeabra.Sim(net, paramSets, env)
sim.Init()
sim.NewRun()
print("Starting run...")
sim.Run(25)
print("Completed!")

mse = np.array(sim.EpochSSE["Output"])

print("Creating plot...")
plt.plot(mse)
plt.title("Training RA25")
plt.xlabel("Epoch")
plt.ylabel("MSE")
print("Saving to file...")
plt.savefig("ra25-py.png")
print("Done")