#include "emer.hpp"

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
