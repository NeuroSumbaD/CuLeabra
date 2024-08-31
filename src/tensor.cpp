#include "tensor.hpp"

// Len returns the total length of elements in the tensor (i.e., the product of
// the shape sizes)
int tensor::Shape::Len() {
    if (Sizes.size()== 0) return 0;

    int o = 1;
    for (int& i: Sizes){
        o *= i;
    }
    return o;
}

tensor::Shape::Shape(std::vector<int>& shape){
    Sizes = shape;
    Strides = RowMajorStrides(shape);
    Names = std::vector<std::string>();
    for (int i = 0; i < shape.size(); i++) Names.push_back("");
}

tensor::Shape::Shape(std::vector<int>& shape, std::vector<std::string>& names) {
    Sizes = shape;
    Strides = RowMajorStrides(shape);
    Names = names;

}

// NewShape returns a new shape object initialized with params.
// If strides is nil, row-major strides will be inferred.
// If names is nil, a slice of empty strings will be created.
tensor::Shape::Shape(std::vector<int>& shape, std::vector<int>& strides, std::vector<std::string>& names) {
    Sizes = shape;

    if (strides.size() != shape.size()) {
        Strides = RowMajorStrides(shape);
    } else {
        Strides = strides;
    }
    
    if (names.size() == shape.size()) {
        Names = names;
    } else {
        Names = std::vector<std::string>();
        for (int i = 0; i < shape.size(); i++) Names.push_back("");
    }
}

// Offset returns the "flat" 1D array index into an element at the given n-dimensional index.
// No checking is done on the length or size of the index values relative to the shape of the tensor.
int tensor::Shape::Offset(std::vector<int> index) {
    int offset = 0;
	for (int i = 0; i < index.size(); i++) {
        int v = index[i];
		offset += v * Strides[i];
	}
	return offset;
}

// Index returns the n-dimensional index from a "flat" 1D array index.
std::vector<int> tensor::Shape::Index(int offset) {
    int nd = Sizes.size();
    std::vector<int> index;
    index.reserve(nd);
	int rem = offset;
	for (int i = (nd - 1); i >= 0; i--) {
		int s = Sizes[i];
		int iv = rem % s;
		rem /= s;
		index[i] = iv;
	}
	return index;
}

tensor::Shape tensor::AddShapes(Shape shape1, Shape shape2) {
    auto & sh1 = shape1.Sizes;
    auto & sh2 = shape2.Sizes;



    auto nsh = std::vector<int>();
    nsh.insert(nsh.end(), sh1.begin(), sh1.end());
    nsh.insert(nsh.end(), sh2.begin(), sh2.end());

    auto nms = std::vector<std::string>();
    nms.insert(nms.end(), shape1.Names.end(), shape1.Names.end());
    nms.insert(nms.end(), shape2.Names.end(), shape2.Names.end());

    return tensor::Shape(nsh, nms);
}

std::vector<int> tensor::RowMajorStrides(std::vector<int> shape) {
    int rem = 1;
    for (int & v: shape){
        rem *= v;
    }

    if (rem == 0) {
        auto strides = std::vector<int>();
        for (int& i: shape){
            strides.push_back(rem);
        }
        return strides;
    } else {
        auto strides = std::vector<int>();
        for (int & v: shape){
            rem /= v;
            strides.push_back(rem);
        }  
        return strides;
    }
}

std::vector<int> tensor::ColMajorStrides(std::vector<int> shape) {
    int total = 1;
    for (int & v: shape){
        if (v==0){ // TODO Check why it was written this way
            auto strides = std::vector<int>();
            for (int & v: shape){
                strides.push_back(total);
            }
        }
    }

    auto strides = std::vector<int>();
    for (int & v: shape){
        strides.push_back(total);
        total *= v;
    }
    return strides;
}
