# Makefile

# Compiler settings
CC := g++
NVCC := nvcc
CFLAGS := -Wall  -std=c++17 -I./include
PYBINDFLAGS := -shared -fPIC $$(python3 -m pybind11 --includes)
#pybind11 module returns the include paths -I/usr/include/python3.12/ -I/usr/lib/python3/dist-packages/pybind11/include
#can also add -O3 to include optimization step 
NVCCFLAGS := -I./include -I/usr/include/python3.12/ -I/usr/include/pybind11/
NVCCPYBINDFLAGS := -Xcompiler -fPIC -shared
LDFLAGS := -lcudart

# Source files
# Directories
SRC_DIR := src
OBJ_DIR := culeabra
INC_DIR := include
TEST_DIR := tests

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TESTS := $(patsubst $(TEST_DIR)/%.cpp,$(TEST_DIR)/%,$(wildcard $(TEST_DIR)/*.cpp))

# SRCS := matrix_multiply.cu matrix_multiply.cpp
# OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := _culeabra
# TARGET := my_matrix_multiplier

all: $(TARGET)

# The automatic variable "$@" is replaced with the file name
# The automatic variable "$^" is replaced with a list of the dependencies
$(TARGET): $(OBJS)
	$(CC) $(PYBINDFLAGS) -o $(OBJ_DIR)/$@$$(python3-config --extension-suffix) $^
#	$(NVCC) $(LDFLAGS) -o $@$$(python3-config --extension-suffix) $^

# %.o: %.cpp
# The automatic variable "$<" is replaced with the first dependency
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.hpp
	$(info making object file: $@)
	$(CC) $(CFLAGS) $(PYBINDFLAGS) -c -o $@ $<

# %.o: %.cu
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu
	$(info making object file: $@)
	$(NVCC) $(CFLAGS) -c -o $@ $<


# UNIT TESTING FOR MODULES
tests: $(TESTS)

$(TEST_DIR)/%: $(TEST_DIR)/%.o $(OBJS)
	$(info making test: $@)
	$(info with dependencies: $^)
	$(CC) $(CFLAGS) -fPIC -o  $@ $^

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -fPIC -c -o  $@ $^

clean:
	rm -f $(OBJS) $(OBJ_DIR)/$(TARGET)*

.PHONY: all clean tests