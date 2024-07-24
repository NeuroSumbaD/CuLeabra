#!/bin/bash

# docker build -t culeabra:v0.1 .
docker run -it --gpus all --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix" \
    -v C:\Users\luisx\Documents\ACT3:/app --name CUDA-Leabra-DevEnv culeabra:v0.1

# TODO: set up a port forward to a server for running the GUI (maybe X11, probably some other app that handles rendering on the browser side)