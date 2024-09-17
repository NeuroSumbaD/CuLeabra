#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>

namespace tensor {

    std::vector<int> RowMajorStrides(std::vector<int> shape);
    std::vector<int> ColMajorStrides(std::vector<int> shape);

    // Shape manages a tensor's shape information, including strides and dimension names
    // and can compute the flat index into an underlying 1D data storage array based on an
    // n-dimensional index (and vice-versa).
    // Per C / Go / Python conventions, indexes are Row-Major, ordered from
    // outer to inner left-to-right, so the inner-most is right-most.
    struct Shape{
        std::vector<int> Sizes; // shape is size per dimension
        std::vector<int> Strides; // stride is offset per dimension 
        std::vector<std::string> Names; // names of each dimension

        Shape();
        Shape(std::vector<int>& shape);
        Shape(std::vector<int>& shape, std::vector<std::string>& names);
        Shape(std::vector<int>& shape, std::vector<int>& strides, std::vector<std::string>& names);
        int Len();
        // std::string String();

        // IsRowMajor returns true if shape is row-major organized:
        // first dimension is the row or outer-most storage dimension.
        // Values *along a row* are contiguous, as you increment along
        // the minor, inner-most (column) dimension.
        // Importantly: ColMajor and RowMajor both have the *same*
        // actual memory storage arrangement, with values along a row
        // (across columns) contiguous in memory -- the only difference
        // is in the order of the indexes used to access this memory.
        bool IsRowMajor();

        // IsColMajor returns true if shape is column-major organized:
        // first dimension is column, i.e., inner-most storage dimension.
        // Values *along a row* are contiguous, as you increment along
        // the major inner-most (column) dimension.
        // Importantly: ColMajor and RowMajor both have the *same*
        // actual memory storage arrangement, with values along a row
        // (across columns) contiguous in memory -- the only difference
        // is in the order of the indexes used to access this memory.
        bool IsColMajor();

        // NumDims returns the total number of dimensions.
        int NumDims();

        int Offset(std::vector<int> index);
        std::vector<int> Index(int offset);

        int DimSize(int i);

        bool IndexIsValid(std::vector<int> idx);
        void SetShape(std::vector<int> sizes, std::vector<std::string> names);
    };
    Shape AddShapes(Shape shape1, Shape shape2);




    // Tensor is the interface for n-dimensional tensors.
    // Per C / Go / Python conventions, indexes are Row-Major, ordered from
    // outer to inner left-to-right, so the inner-most is right-most.
    // It is implemented by the Base and Number generic types specialized
    // by different concrete types: float64, float32, int, int32, byte,
    // string, bits (bools).
    // For float32 and float64 values, use NaN to indicate missing values.
    // All of the data analysis and plot packages skip NaNs.
    template <typename T = float>
    struct Tensor {
        std::vector<T> Values;
        Shape Shp;
        std::map<std::string, std::string> Meta;

        Tensor(std::vector<int>& shape):Shp(shape){
            int nln = Shp.Len();
            Values = std::vector<T>();
            Values.reserve(nln);
        };        
        Tensor(Shape &shape){
            Shp = shape;

            int nln = Shp.Len();
            Values = std::vector<T>();
            Values.reserve(nln);
        };

        // Shape returns a pointer to the shape that fully parametrizes the tensor shape
        Shape* GetShape(){return &Shp;};

        // Len returns the number of elements in the tensor (product of shape dimensions).
        int Len(){return Shp.Len();};

        // NumDims returns the number of dimensions of the tensor.
        int NumDims(){return Shp.Sizes.size();};

        // Dim returns the size of the given dimension
        int Dim(int index){return Shp.Sizes[index];};

        // DimNames returns the string slice of dimension names
        virtual std::vector<std::string> DimNames(){return Shp.Names;};

        // DimName returns the name of the i-th dimension.
        std::string DimName(int index){return Shp.Names[index];};

        int DimSize(int i){return Shp.Sizes[i];};

        // SetShape sets the shape parameters of the tensor, and resizes backing storage appropriately.
        // existing RowMajor or ColMajor stride preference will be used if strides is nil, and
        // existing names will be preserved if nil
        // virtual void SetShape(std::vector<int> shape, std::vector<int> strides, std::vector<int> names);

        // SetMetaData sets a key=value meta data (stored as a map[string]string).
        // For TensorGrid display: top-zero=+/-, odd-row=+/-, image=+/-,
        // min, max set fixed min / max values, background=color
        void SetMetaData(std::string key, std::string val){Meta[key] = val;};

        // MetaData retrieves value of given key
        std::string MetaData(std::string key){
            if (Meta.count(key) == 0) {
                std::cerr << "Tensor metadata does not have key: " + key << std::endl;
            } else {
                return Meta[key];
            };
        };

        // Set index to value
        void SetValue(int index, T val){
            Values[index] = val;
        }

        // Set all indices to value
        void SetAll(T val){
            for (size_t i = 0; i < Values.size(); ++i) {
                Values[i] = val;
            }
        }
    };

    
    typedef Tensor<int> Int32;
    typedef Tensor<bool> Bits; // uses vector<bool> as an efficient bit array (in the Values vector)

    std::tuple<int, int, int, int> Projection2DShape(Shape &shp, bool oddRow);
    int Projection2DIndex(Shape &shp, bool oddRow, int row, int col);
} // namespace tensor