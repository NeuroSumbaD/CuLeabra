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

bool tensor::Shape::IsRowMajor(){
    auto strides = RowMajorStrides(Sizes);
    return (strides == Strides);
}

tensor::Shape::Shape(){
    Sizes = std::vector<int>();
    Strides = std::vector<int>();
    Names = std::vector<std::string>();
}

tensor::Shape::Shape(std::vector<int> &shape) {
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

bool tensor::Shape::IsColMajor(){
    auto strides = ColMajorStrides(Sizes);
    return (strides == Strides);
}

int tensor::Shape::NumDims() {
    return Sizes.size();
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

int tensor::Shape::DimSize(int i) {
    return Sizes[i];
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

// Projection2DShape returns the size of a 2D projection of the given tensor Shape,
// collapsing higher dimensions down to 2D (and 1D up to 2D).
// For any odd number of dimensions, the remaining outer-most dimension
// can either be multipliexed across the row or column, given the oddRow arg.
// Even multiples of inner-most dimensions are assumed to be row, then column.
// rowEx returns the number of "extra" (higher dimensional) rows
// and colEx returns the number of extra cols
std::tuple<int, int, int, int> tensor::Projection2DShape(Shape &shp, bool oddRow) {
    if (shp.Len() == 0) {
		return std::tuple<int, int, int, int>(1, 1, 0, 0);
	}
	int nd = shp.NumDims();
	switch (nd) {
	case 1:
		if (oddRow) {
			return std::tuple<int, int, int, int>(shp.DimSize(0), 1, 0, 0);
		} else {
			return std::tuple<int, int, int, int>(1, shp.DimSize(0), 0, 0);
		}
        break;
	case 2:
		return std::tuple<int, int, int, int>(shp.DimSize(0), shp.DimSize(1), 0, 0);
        break;
	case 3:
		if (oddRow) {
			return std::tuple<int, int, int, int>(shp.DimSize(0) * shp.DimSize(1), shp.DimSize(2), shp.DimSize(0), 0);
		} else {
			return std::tuple<int, int, int, int>(shp.DimSize(1), shp.DimSize(0) * shp.DimSize(2), 0, shp.DimSize(0));
		}
        break;
	case 4:
		return std::tuple<int, int, int, int>(shp.DimSize(0) * shp.DimSize(2), shp.DimSize(1) * shp.DimSize(3), shp.DimSize(0), shp.DimSize(1));
	case 5:
		if (oddRow) {
			return std::tuple<int, int, int, int>(shp.DimSize(0) * shp.DimSize(1) * shp.DimSize(3), shp.DimSize(2) * shp.DimSize(4), shp.DimSize(0) * shp.DimSize(1), 0);
		} else {
			return std::tuple<int, int, int, int>(shp.DimSize(1) * shp.DimSize(3), shp.DimSize(0) * shp.DimSize(2) * shp.DimSize(4), 0, shp.DimSize(0) * shp.DimSize(1));
		}
	}
    return std::tuple<int, int, int, int>(1, 1, 0, 0);
}

// Projection2DIndex returns the flat 1D index for given row, col coords for a 2D projection
// of the given tensor shape, collapsing higher dimensions down to 2D (and 1D up to 2D).
// For any odd number of dimensions, the remaining outer-most dimension
// can either be multipliexed across the row or column, given the oddRow arg.
// Even multiples of inner-most dimensions are assumed to be row, then column.
int tensor::Projection2DIndex(Shape &shp, bool oddRow, int row, int col) {
    int nd = shp.NumDims();

    int nyy;
    int ny;
    int yyy;
    int yy;
    int y;
    int nx;
    int xx;
    int x;

	switch (nd) {
        case 1:
            if (oddRow) {
                return row;
            } else {
                return col;
            }
            break;
        case 2:
            return shp.Offset({row, col});
            break;
        case 3:
            if (oddRow) {
                ny = shp.DimSize(1);
                yy = row / ny;
                y = row % ny;
                return shp.Offset({yy, y, col});
                break;
            } else {
                nx = shp.DimSize(2);
                xx = col / nx;
                x = col % nx;
                return shp.Offset({xx, row, x});
                break;
            }
        case 4:
            ny = shp.DimSize(2);
            yy = row / ny;
            y = row % ny;
            nx = shp.DimSize(3);
            xx = col / nx;
            x = col % nx;
            return shp.Offset({yy, xx, y, x});
            break;
        case 5:
            // todo: oddRows version!
            nyy = shp.DimSize(1);
            ny = shp.DimSize(3);
            yyy = row / (nyy * ny);
            yy = row % (nyy * ny);
            y = yy % ny;
            yy = yy / ny;
            nx = shp.DimSize(4);
            xx = col / nx;
            x = col % nx;
            return shp.Offset({yyy, yy, xx, y, x});
            break;
        default:
            return 0;
	}
	return 0;
}
