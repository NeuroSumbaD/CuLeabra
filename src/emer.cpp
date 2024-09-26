#include "emer.hpp"

namespace emer{
	const std::vector<std::string> LayerDimNames2D = {"Y", "X"};
	const std::vector<std::string> LayerDimNames4D = {"PoolY", "PoolX", "NeurY", "NeurX"};
} // namespace emer


emer::Network::Network(std::string name, std::string weightsFile, int randSeed):
	Name(name), WeightsFile(weightsFile), LayerNameMap(), LayerClassMap(), MinPos(), MaxPos(), MetaData(), Rand(), RandSeed(randSeed){}

// UpdateLayerMaps updates the LayerNameMap and LayerClassMap.
void emer::Network::UpdateLayerMaps() {
    int nl = NumLayers();
	for (int li = 0; li < nl; li++) {
		emer::Layer *ly = EmerLayer(li);
		std::string lnm = ly->StyleName();
		LayerNameMap[lnm] = ly;
		std::vector<std::string> cls = strings::split(ly->StyleClass(), ' ');
		for(std::string &cl: cls) {;
			std::vector<std::string> &ll = LayerClassMap[cl];
			ll.push_back(lnm);
			// LayerClassMap[cl] = ll; //REDUNDANT
            // TODO: Check if the LayerClassMap is kept accurately
		}
	}
}

// EmerLayerByName returns a layer by looking it up by name.
// returns nullptr if layer is not found.
emer::Layer *emer::Network::LayerByName(std::string name) {
	if (LayerNameMap.size() != NumLayers()) {
		UpdateLayerMaps();
	}
	if (LayerNameMap.count(name)>0) {
		Layer *ly= LayerNameMap[name];
		return ly;
	}
    return nullptr;
}

// EmerPathByName returns a path by looking it up by name.
// Paths are named SendToRecv = sending layer name "To" recv layer name.
// returns error message if path is not found.
emer::Path *emer::Network::PathByName(std::string name) {
	int ti = name.find("To");
	if (ti == std::string::npos) {
		return nullptr;
	}
	std::string sendNm = name.substr(0,ti);
	std::string recvNm = name.substr(ti+2);
	Layer *send = LayerByName(sendNm);
	if (send == nullptr) {
		return nullptr;
	}
	Layer *recv = LayerByName(recvNm);
	if (recv == nullptr) {
		return nullptr;
	}
	Path *path = recv->RecvPathBySendName(sendNm);
	return path;
}

// LayersByClass returns a list of layer names by given class(es).
// Lists are compiled when network Build() function called,
// or now if not yet present.
// The layer Type is always included as a Class, along with any other
// space-separated strings specified in Class for parameter styling, etc.
// If no classes are passed, all layer names in order are returned.
std::vector<std::string> emer::Network::LayersByClass(std::vector<std::string> classes) {
	if (LayerClassMap.size() == 0) {
		UpdateLayerMaps();
	}
	std::vector<std::string> nms = std::vector<std::string>();
	int nl = NumLayers();
	if (classes.size() == 0) {
		for (int li = 0; li < nl; li++) {
			Layer *ly = EmerLayer(li);
			if (ly->Off) {
				continue;
			}
			nms.push_back(ly->Name);
		}
		return nms;
	}
	for (std::string &lc: classes) {
		nms.insert(nms.end(), LayerClassMap[lc].begin(), LayerClassMap[lc].end());
	}
	// only get unique layers
	std::vector<std::string> layers = std::vector<std::string>();
	std::map<std::string, bool> has = std::map<std::string, bool>();
	for(std::string &nm: nms) {
		if (has[nm]) {
			continue;
		}
		layers.push_back(nm);
		has[nm] = true;
	}
	if (layers.size() == 0) {
		std::cerr << "No Layers found for query: " << strings::join(classes, " ") << std::endl;
	}
	return layers;
}

// LayoutLayers computes the 3D layout of layers based on their relative
// position settings.
void emer::Network::LayoutLayers() {
	// en := nt.EmerNetwork
	int nlay = NumLayers();
	for (int index = 0; index < 5; index++) {
		emer::Layer *lstly = nullptr;
		for (int li = 0; li < nlay; li++) {
			emer::Layer *ly = EmerLayer(li);
			emer::Layer *oly = nullptr;
			if (lstly != nullptr && ly->Pos.Rel == relpos::NoRel) {
				if (ly->Pos.Pos.X != 0 || ly->Pos.Pos.Y != 0 || ly->Pos.Pos.Z != 0) {
					// Position has been modified, don't mess with it.
					continue;
				}
				oly = lstly;
				ly->Pos = relpos::Pos{Rel: relpos::Above, XAlign: relpos::Middle, YAlign: relpos::Front, Other: lstly->Name};
			} else {
				if (ly->Pos.Other != "") {
					emer::Layer *olyi = LayerByName(ly->Pos.Other);
					oly = olyi;
				} else if (lstly != nullptr) {
					oly = lstly;
					ly->Pos = relpos::Pos{Rel: relpos::Above, XAlign: relpos::Middle, YAlign: relpos::Front, Other: lstly->Name};
				}
			}
			if (oly != nullptr) {
				ly->Pos.SetPos(oly->Pos.Pos, oly->DisplaySize(), ly->DisplaySize());
			}
			lstly = ly;
		}
	}
	LayoutBoundsUpdate();
}

// layoutBoundsUpdate updates the Min / Max display bounds for 3D display.
void emer::Network::LayoutBoundsUpdate() {
	// en := nt.EmerNetwork
	int nlay = NumLayers();
	math::Vector3 mn = math::Vector3(math::Infinity);
	math::Vector3 mx = math::Vector3();
	for (int li = 0; li < nlay; li++) {
		Layer *ly = EmerLayer(li);
		math::Vector2 sz = ly->DisplaySize();
		math::Vector3 ru = ly->Pos.Pos;
		ru.X += sz.X;
		ru.Y += sz.Y;
		mn.SetMin(ly->Pos.Pos);
		mx.SetMax(ru);
	}
	MinPos = mn; // TODO: pull request Emergent to correct a mistake in this line
	MaxPos = mx;
}

// ApplyParams applies given parameter style Sheet to layers and paths in this network.
// Calls UpdateParams to ensure derived parameters are all updated.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// it always prints a message if a parameter fails to be set.
// returns true if any params were set, and error if there were any errors.
bool emer::Network::ApplyParams(params::Sheet &pars, bool setMsg) {
	bool applied = true;
	std::vector<std::string> errs;
	// en := nt.EmerNetwork
	int nlay = NumLayers();
	for (int li = 0; li < nlay; li++) {
		emer::Layer &ly = *EmerLayer(li);
		bool app = ly.ApplyParams(pars, setMsg);
		if (!app) {
			applied = false;
			break;
		} else {
			if (!setMsg){
				std::cout << "Params set on layer " << ly.Name << std::endl;
			}
		}
		// if err != nil {
		// 	errs = append(errs, err)
		// }
	}
	return applied;
}

// Applies a sheet from the Sets of parameter sheets provided. Defaults to the 'base'
// sheet if the name is not provided. If name="ALL" then all the sheets are applied.
bool emer::Network::ApplyParams(params::Sets &pars, bool setMsg, std::string name) {
	if (name == "ALL") {
		for (auto &[name,sheet]: pars.sheets) {
			bool app = ApplyParams(sheet, setMsg);
			if (!app){
				std::cerr << "Parameter sheet '" << name << "' not applied." << std::endl;
				return false;
			}
		}
		return true;
	}
    return ApplyParams(pars.sheets[name], setMsg);
}

void emer::Network::SetRandSeed(int seed){
	RandSeed = seed;
	ResetRandSeed();
}

void emer::Network::ResetRandSeed(){
	Rand.NewSeed(RandSeed);
}

void emer::Path::AddClass(std::vector<std::string> classes){
    std::string newClasses= "";
    for (std::string cls: classes) {
        if (Class == ""){
            newClasses += cls;
        }
        else {
            newClasses += " " + cls;
        }
    };
    Class += newClasses;
}

void emer::Path::SetParam(std::string path, std::string val) {
    
}

// ApplyParams applies given parameter style Sheet to this pathway.
// Calls UpdateParams if anything set to ensure derived parameters are all updated.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// it always prints a message if a parameter fails to be set.
// returns true if any params were set, and error if there were any errors.
bool emer::Path::ApplyParams(params::Sheet &pars, bool setMsg) {
	std::any obj((StylerObject*)this);
    bool app = pars.Apply(obj, setMsg);
	// note: must use EmerPath to get to actual Path, which then uses Styler interface
	// to return the Params struct.
	if (app) {
		UpdateParams();
	}
	return app;
}

emer::Layer::Layer(std::string name, int index, std::vector<int> shape):Name(name), Off(false), Shape(shape), Pos(), Index(0), SampleIndexes(), SampleShape(shape), MetaData(){
	// InitParamMaps();
}

std::string emer::Layer::StyleType() {
	return "Layer";
}

std::string emer::Layer::StyleClass() {
	return this->TypeName() + " " + this->Class;
}

std::string emer::Layer::StyleName() {
	return this->Name;
}

std::string emer::Layer::Label() {
	return Name;
}

bool emer::Layer::Is2D() {
	return Shape.NumDims() == 2;
}

bool emer::Layer::Is4D() {
	return Shape.NumDims() == 4;
}

int emer::Layer::NumUnits() {
	return Shape.Len();
}

std::tuple<std::vector<int>, bool> emer::Layer::Index4DFrom2D(int x, int y) {
	tensor::Shape &lshp = Shape;
	int nux = lshp.DimSize(3);
	int nuy = lshp.DimSize(2);
	int ux = x % nux;
	int uy = y % nuy;
	int px = x / nux;
	int py = y / nuy;
	std::vector<int> idx = {py, px, uy, ux};
	if (!lshp.IndexIsValid(idx)) {
		return std::tuple<std::vector<int>, bool>(NULL, false);
	}
    return std::tuple<std::vector<int>, bool>(idx, true);
}

// PlaceRightOf positions the layer to the right of the other layer,
// with given spacing, using default YAlign = Front alignment.
void emer::Layer::PlaceRightOf(Layer &other, float space) {
	Pos.SetRightOf(other.Name, space);
}

// PlaceBehind positions the layer behind the other layer,
// with given spacing, using default XAlign = Left alignment.
void emer::Layer::PlaceBehind(Layer &other, float space) {
	Pos.SetBehind(other.Name, space);
}

// PlaceAbove positions the layer above the other layer,
// using default XAlign = Left, YAlign = Front alignment.
void emer::Layer::PlaceAbove(Layer &other, float space) {
	Pos.SetAbove(other.Name);
}

// DisplaySize returns the display size of this layer for the 3D view.
// see Pos field for general info.
// This is multiplied by the Pos.Scale factor to rescale
// layer sizes, and takes into account 2D and 4D layer structures.
math::Vector2 emer::Layer::DisplaySize() {
	if (Pos.Scale == 0) {
		Pos.Defaults();
	}
	
	math::Vector2 sz;
	if (Is2D()){
		sz = math::Vector2(float(Shape.DimSize(1)), float(Shape.DimSize(0))); // Y, X
	}
	if (Is4D()){
		// note: pool spacing is handled internally in display and does not affect overall size
		sz = math::Vector2(float(Shape.DimSize(1)*Shape.DimSize(3)), float(Shape.DimSize(0)*Shape.DimSize(2))); // Y, X
	} else {
		sz = math::Vector2(float(Shape.Len()), 1);
	}
	
	return sz.MulScalar(Pos.Scale);
}

// SetShape sets the layer shape and also uses default dim names.
void emer::Layer::SetShape(std::vector<int> shape) {
	std::vector<std::string> dnms;
	if (shape.size() == 2) {
		dnms = emer::LayerDimNames2D;
	} else if (shape.size() == 4) {
		dnms = emer::LayerDimNames4D;
	}
	Shape.SetShape(shape, dnms);
}

// SetSampleIndexesShape sets the SampleIndexes,
// and SampleShape and as list of dimension sizes,
// for a subset sample of units to represent the entire layer.
// This is critical for large layers that are otherwise unwieldy
// to visualize and for computationally-intensive statistics.
void emer::Layer::SetSampleIndexesShape(std::vector<int> idxs, std::vector<int> shape) {
	SampleIndexes = idxs;
	std::vector<std::string> dnms;
	if (shape.size() == 2) {
		dnms = LayerDimNames2D;
	} else if (shape.size() == 4) {
		dnms = LayerDimNames4D;
	}
	SampleShape.SetShape(shape, dnms);
}

// GetSampleShape returns the shape to use for representative units.
tensor::Shape emer::Layer::GetSampleShape() {
    int sz = SampleIndexes.size();
	if (sz == 0) {
		return Shape;
	}
	if (SampleShape.Len() != sz) {
		SampleShape.SetShape({sz}, {});
	}
	return SampleShape;
}

// NSubPools returns the number of sub-pools of neurons
// according to the shape parameters.  2D shapes have 0 sub pools.
// For a 4D shape, the pools are the first set of 2 Y,X dims
// and then the neurons within the pools are the 2nd set of 2 Y,X dims.
int emer::Layer::NumPools() {
	if (Shape.NumDims() != 4) {
		return 0;
	}
	return Shape.DimSize(0) * Shape.DimSize(1);
}

// Layer2DSampleIndexes returns neuron indexes and corresponding 2D shape
// for the representative neurons within a large 2D layer, for passing to
// [SetSampleIndexesShape].  These neurons are used for the raster plot
// in the GUI and for computing PCA, among other cases where the full set
// of neurons is problematic. The lower-left corner of neurons up to
// given maxSize is selected.
std::tuple<std::vector<int>, std::vector<int>> emer::Layer::Layer2DSampleIndexes(Layer &ly, int maxSize) {
	// lb := ly.AsEmer()
	tensor::Shape &sh = ly.Shape;
	int my = std::min(maxSize, sh.DimSize(0));
	int mx = std::min(maxSize, sh.DimSize(1));
	std::vector<int> shape = {my, mx};
	std::vector<int> idxs(my*mx);
	int i = 0;
	for (int y = 0; y < my; y++) {
		for (int x = 0; x < mx; x++) {
			idxs[i] = sh.Offset({y, x});
			i++;
		}
	}
    return std::tuple<std::vector<int>, std::vector<int>>(idxs, shape);
}

// RecvPathBySendName returns the receiving Path with given
// sending layer name (the first one if multiple exist).
emer::Path *emer::Layer::RecvPathBySendName(std::string sender) {
	for (int pi = 0; pi < NumRecvPaths(); pi++) {
		Path &pt = *RecvPath(pi);
		if (pt.SendLayer()->StyleName() == sender) {
			return &pt;
		}
	}
	std::cerr << "Sending layer named: "<< sender << "not found in list of receiving pathways" << std::endl;
	// return nil, fmt.Errorf("sending layer named: %s not found in list of receiving pathways", sender)
    return nullptr;
}

// SendPathByRecvName returns the sending Path with given
// recieving layer name (the first one if multiple exist).
emer::Path *emer::Layer::SendPathByRecvName(std::string recv) {
    for (int pi = 0; pi < NumRecvPaths(); pi++) {
		Path &pt = *SendPath(pi);
		if (pt.RecvLayer()->StyleName() == recv) {
			return &pt;
		}
	}
	std::cerr << "Receiving layer named: "<< recv << "not found in list of receiving pathways" << std::endl;
    return nullptr;
}

// RecvPathBySendNameType returns the receiving Path with given
// sending layer name, with the given type name
// (the first one if multiple exist).
emer::Path *emer::Layer::RecvPathBySendNameType(std::string sender, std::string typeName) {
    for (int pi = 0; pi < NumRecvPaths(); pi++) {
		Path &pt = *RecvPath(pi);
		if (pt.SendLayer()->StyleName() == sender && pt.TypeName() == typeName) {
			return &pt;
		}
	}
	std::cerr << "Sending layer named: "<< sender << ", type " << typeName << "not found in list of receiving pathways" << std::endl;
    return nullptr;
}

// SendPathByRecvNameType returns the sending Path with given
// recieving layer name, with the given type name
// (the first one if multiple exist).
emer::Path *emer::Layer::SendPathByRecvNameType(std::string recv, std::string typeName) {
	for (int pi = 0; pi < NumRecvPaths(); pi++) {
		Path &pt = *SendPath(pi);
		if (pt.RecvLayer()->StyleName() == recv && pt.TypeName() == typeName) {
			return &pt;
		}
	}
	std::cerr << "Receving layer named: "<< recv << ", type " << typeName << "not found in list of receiving pathways" << std::endl;
    return nullptr;
}

// SetParam sets parameter at given path to given value.
// returns error if path not found or value cannot be set.
void emer::Layer::SetParam(std::string path, std::string val) {
	params::SetParam(GetStyleObject(), path, val);
}

// ApplyParams applies given parameter style Sheet to this layer and its recv pathways.
// Calls UpdateParams on anything set to ensure derived parameters are all updated.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// it always prints a message if a parameter fails to be set.
// returns true if any params were set, and error if there were any errors.
bool emer::Layer::ApplyParams(params::Sheet &pars, bool setMsg) {
    bool applied = false;
	// std::vector<std::string> errs;
	std::any obj((StylerObject*)this);
	bool app = pars.Apply(obj, setMsg); // essential to go through AxonLay
	if (app) {
		UpdateParams();
		applied = true;
	}
	// if err != nil {
	// 	errs = append(errs, err)
	// }
	// el := EmerLayer
	for (int pi = 0; pi < NumRecvPaths(); pi++) {
		Path &pt = *RecvPath(pi);
		app = pt.ApplyParams(pars, setMsg);
		if (app) {
			applied = true;
		}
		// if err != nil {
		// 	errs = append(errs, err)
		// }
	}
	return applied; //, errors.Join(errs...)
}

emer::Path::Path(std::string name, std::string cls):
	Name(name), Class(cls), Info(), Notes() {
	Pattern = nullptr;
	// InitParamMaps();
}

std::string emer::Path::StyleType() {
	return "Path";
}

std::string emer::Path::StyleClass() {
	return TypeName() + " " + Class;
}

std::string emer::Path::StyleName() {
	return Name;
}

std::string emer::Path::Label() {
	return Name;
}


