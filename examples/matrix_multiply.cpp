// matrix_multiply.cpp
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>


namespace py = pybind11;

float multiply_matrices(float* A, float* B, float* C, int size);
// void multiply_matrices(float* A, float* B, float* C, int size){
//     C[0] = A[0] * B[0];
//  };

PYBIND11_MODULE(matrix_multiply, m) {
    m.doc() = "This module exposes a cuda matrix multiplier function to python";
    m.def("multiply_matrices", &multiply_matrices, "A function that multiplies matrices in CUDA");
    // m.def("multiply_matrices",  {
    //     py::buffer_info buf_A = A.request();
    //     py::buffer_info buf_B = B.request();
    //     py::buffer_info buf_C = C.request();
    //     py::buffer_info buf_size = size.request();

    //     if (buf_A.size != buf_B.size) {
    //         throw std::runtime_error("Input arrays must have the same size.");
    //     }

    //     std::vector<float> result(buf_A.size);
    //     multiply_matrices(static_cast<float*>(buf_A.ptr), static_cast<float*>(buf_B.ptr),
    //                       result.data(), buf_A.size);

    //     return py::array_t<float>(buf_A.size, result.data());
    // });
}