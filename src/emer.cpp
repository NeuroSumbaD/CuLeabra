#include "emer.hpp"

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
		emer::Layer *lstly;
		for (int li = 0; li < nlay; li++) {
			emer::Layer *ly = EmerLayer(li);
			emer::Layer *oly;
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
    bool app = pars.Apply((params::StylerObject *)this, setMsg);
	// note: must use EmerPath to get to actual Path, which then uses Styler interface
	// to return the Params struct.
	if (app) {
		UpdateParams();
	}
	return app;
}

emer::Layer::Layer(std::string name, int index, std::vector<int> shape):Name(name), Off(true), Shape(shape), Pos(), Index(0), SampleIndexes(), SampleShape(shape), MetaData(){}

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

emer::Path::Path(std::string name, std::string cls):
	Name(name), Class(cls), Info(), Notes() {
	Pattern = nullptr;
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


