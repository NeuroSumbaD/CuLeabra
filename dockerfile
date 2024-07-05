FROM nvidia/cuda:12.5.1-devel-ubuntu24.04
# FROM ubuntu:24.04

# Update package lists and install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        git \
        unzip \
        tk-dev \
        python3 \
        python3-pip \
        python3-dev \
        pybind11-dev \
        python3-pybind11 \
        python3-tk \
        # cmake \
        # libsm6 \
        # libxext6 \
        gcc \
        g++ \
        nlohmann-json3-dev
        # wget \
        # && rm -rf /var/lib/apt/lists/*

        
# Install CUDA Toolkit (adjust version as needed)
# RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-keyring_1.1-1_all.deb && \
#     dpkg -i cuda-keyring_1.1-1_all.deb && \
#     apt-get update && \
#     apt-get -y install cuda-toolkit-12-5 && \
#     rm ./cuda-keyring_1.1-1_all.deb
# Set environment variables for CUDA
# ENV PATH="/usr/local/cuda/bin:${PATH}"
# ENV LD_LIBRARY_PATH="/usr/local/cuda/lib64:${LD_LIBRARY_PATH}"

# Remove apt lists to shrink container size
RUN rm -rf /var/lib/apt/lists/*
# Clean up
RUN apt-get clean
        
# Optional: Install Python packages (if needed)
RUN rm /usr/lib/python*/EXTERNALLY-MANAGED && \
    # python3 -m ensurepip && \
    pip3 install numpy matplotlib 'uvicorn[standard]' FastAPI
# RUN pip3 install numpy pybind11


# Copy your C++ code into the container (adjust path as needed)
# DO NOT COPY DURING DEVELOPMENT OR IMAGE WILL BUILD VERY SLOWLY WITH EACH CHANGE
# COPY ./src /app

# Create a working directory
WORKDIR /app


# Build your CUDA-accelerated C++ program
# RUN mkdir build && \
#     cd build && \
#     cmake .. && \
#     make

# Set the entry point (modify as needed)
CMD ["bash"]

# Optional: Expose ports or add other configuration


# Optional: Add any other dependencies

