# Makefile

# Compiler settings
CC := g++
NVCC := nvcc
CFLAGS := -std=c++20 -I./include -Wall
PYLIBS := $(shell python3-config --ldflags) -lpython3.12
PYBINDINCLUDES := $(shell python3 -m pybind11 --includes)
PYBINDFLAGS := -shared -fPIC
DEBUGFLAGS := -g3 -O0
#pybind11 module returns the include paths -I/usr/include/python3.12/ -I/usr/lib/python3/dist-packages/pybind11/include
#can also add -O3 to include optimization step 
NVCCFLAGS := -I./include
NVCCPYBINDFLAGS := -Xcompiler -fPIC -shared $(shell python3-config --ldflags) -lpython3.12
LDFLAGS := -lcudart

# Source files
# Directories
SRC_DIR := src
OBJ_DIR := culeabra
INC_DIR := include
TEST_DIR := tests
PYLIB_SUFFIX := $(shell python3-config --extension-suffix)

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TESTS := $(patsubst $(TEST_DIR)/%.cpp,$(TEST_DIR)/%,$(wildcard $(TEST_DIR)/*.cpp))

# SRCS := matrix_multiply.cu matrix_multiply.cpp
# OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := _culeabra$(PYLIB_SUFFIX)
# TARGET := my_matrix_multiplier

all: $(TARGET)

# The automatic variable "$@" is replaced with the file name
# The automatic variable "$^" is replaced with a list of the dependencies
$(TARGET): $(OBJS)
	$(CC) $(DEBUGFLAGS) $(PYBINDFLAGS) $(PYBINDINCLUDES) -o $(OBJ_DIR)/$@ $^ $(PYLIBS)
#	$(NVCC) $(LDFLAGS) -o $@$$(python3-config --extension-suffix) $^

# %.o: %.cpp
# The automatic variable "$<" is replaced with the first dependency
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.hpp
	$(info making object file: $@)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(PYBINDFLAGS) $(PYBINDINCLUDES) -c -o $@ $< $(PYLIBS)

# %.o: %.cu
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu
	$(info making object file: $@)
	$(NVCC) $(CFLAGS) $(DEBUGFLAGS) $(NVCCPYBINDFLAGS) $(PYBINDINCLUDES) -c -o $@ $< $(PYLIBS)


# UNIT TESTING FOR MODULES
tests: $(TESTS)

$(TEST_DIR)/%: $(TEST_DIR)/%.o $(OBJS)
	$(info making test: $@)
	$(info with dependencies: $^)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(PYBINDINCLUDES) -fPIC -o  $@ $^ $(PYLIBS)

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(PYBINDINCLUDES) -fPIC -c -o  $@ $^ $(PYLIBS)

clean:
	rm -f $(OBJS) $(OBJ_DIR)/$(TARGET)*

.PHONY: all clean tests