import asyncio
import uvicorn

from fastapi import FastAPI, Query
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

import numpy as np

import time

# TODO: Allow multiple display intervals:
## Streaming in regular intervals (client-side controlled)
## Streaming at specific time-steps of simulation time


app = FastAPI()

NETS = {}

dummyNetworkSpec = {
    "name": "ExampleNet",
    "layers": {
        "Input": {
            "shape": [10,5],
            "projections": ["layer2"],
        },
        "Hidden1": {
            "shape": [5,10],
            "projections": ["layer3", "layer1"],
        },
        "Hidden2": {
            "shape": [5,3],
            "projections": ["layer3", "layer1"],
        },
        "Output": {
            "shape": [5,],
            "projections": ["layer2"],
        },
    }
}
NETS[dummyNetworkSpec["name"]] = dummyNetworkSpec # append to global net dict

dummyData = {
        "Input": np.random.rand(10,5).tolist(),
        "Hidden1": np.random.rand(5,10).tolist(),
        "Hidden2": np.random.rand(5,3).tolist(),
        "Output": np.random.rand(5).tolist(),
    }
    
# Serve static files from the "static" directory
app.mount("/static", StaticFiles(directory="static"), name="static")
app.mount("/scripts", StaticFiles(directory="scripts"), name="scripts")

@app.get("/")
async def read_root():
    return FileResponse("static/index.html")

@app.post("/start_simulation")
async def start_simulation():
    # Your logic to start the neural network simulation
    return {"message": "Simulation started"}

def generate_random_data():
    # Replace this with your actual data fetching logic
    for key in dummyData:
        arr = np.array(dummyData[key])
        arr += 0.1*(np.random.rand(*arr.shape)-0.5)
        dummyData[key] = arr.tolist()
    return dummyData

@app.get("/get_activity")
async def get_activity():
    # Simulate data fetching with a small delay
    # time.sleep(20e-3)
    return generate_random_data()

@app.get("/get_net")
async def get_net(net: str = Query(None)):
    # TODO: Handle error case when the netname doesn't exist
    # TODO: Handle case where user requests the data for a specific net
    if net: # if the fetch includes a netname (ex: fetch('/get_activity?net=netname') )
        try:
            netSpec = NETS[net]
        except KeyError as error:
            return error # TODO: CHECK WHAT HAPPENS
    else:
        netSpec = NETS["ExampleNet"]
            
    return netSpec

async def main():
    config = uvicorn.Config(app, port=8000, log_level="info", reload=True)
    server = uvicorn.Server(config)
    await server.serve()

if __name__ == "__main__":
    asyncio.run(main())
