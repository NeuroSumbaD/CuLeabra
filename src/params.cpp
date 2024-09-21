#include "params.hpp"
#include <stdexcept>
#include <typeinfo>
#include <iostream>
#include "strings.hpp"

// PathAfterType returns the portion of a path string after the initial
// type, e.g., Layer.Acts.Kir.Gbar -> Acts.Kir.Gbar
std::string params::PathAfterType(std::string path) {
    std::vector<std::string> s = strings::split(path, ','); //copy
    s.erase(s.begin()); //drop first element
    return strings::join(s, ".");
}

// Template for supporting any to string conversion
template <typename T>
std::string anyToString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


std::string params::Params::ParamByName(std::string name) {
    if (params.find(name) == params.end()){
        std::string err = "params::Params: parameter named " + name + " not found";
        throw std::invalid_argument(err);
    }
    else {
        return params[name];
    }
}

void params::Params::SetByName(std::string name, std::string value) {
    params[name] = value;
}

// Path returns the second part of the path after the target type,
// indicating the path to the specific parameter being set.
// e.g., Layer.Acts.Kir.Gbar -> Acts.Kir.Gbar
std::string params::Params::Path(std::string path) {
    return params::PathAfterType(path);
}

// TargetType returns the first part of the path, indicating what type of
// object the params apply to.  Uses the first item in the map (which is random)
// everything in the map must have the same target.
std::string params::Params::TargetType() {
    for (const auto& [key, value] : params) {
        return strings::split(key, '.')[0];
    }
    return "";
}

// Apply applies all parameter values to given object.
// Object must already be the appropriate target type based on
// the first element of the path (see TargetType method).
// If setMsg is true, then it will log a confirmation that the parameter
// was set (it always prints an error message if it fails to set the
// parameter at given path, and returns error if so).
std::string params::Params::Apply(std::any obj, bool setMsg) {
    std::string objNm = "";
    try {
        params::StylerObject* sty = std::any_cast<params::StylerObject*>(obj);
        std::string objNm = sty->StyleName();
    } catch (const std::bad_any_cast& e) {
    }

    std::vector<std::string> errs;

    for (auto& [key, value] : params) {
        std::string path = Path(key);

        try {
            params::Hypers* hv = std::any_cast<params::Hypers*>(obj);
            if (hv->map.count(path)){
                hv->map[path].map["Val"] = value;
            }
            else {
                hv->map[path] = params::HyperValues{map: {{"Val", value}}};
            }
            continue;
        }
        catch (const std::bad_any_cast& e) {
            std::string err = params::SetParam(obj, path, value);
            if (err == ""){
                if (setMsg){
                    std::string errStr = objNm + " Set param path: " + key +
                        " to value: " + value;
                    std::cout << errStr << std::endl;
                }
            }
            else {
                errs.push_back(err);
            }
        }
    }
    return strings::join(errs, "");
}

// JSONString returns hyper values as a JSON formatted string
std::string params::HyperValues::JSONString() const {
    // Create a new JSON object from the map
    json reconstructedJson;
    for (const auto& [key, value] : map) {
        reconstructedJson[key] = value;
    }
    // Convert to a JSON string
    std::string jsonStr = reconstructedJson.dump();
    return jsonStr;
}

// SetJSONString sets from a JSON_formatted string
void params::HyperValues::SetJSONString(std::string str) {
    // Parse the JSON string
    json parsedJson = json::parse(str);

    // Convert to a map<string, string>
    for (const auto& [key, value] : parsedJson.items()) {
        map[key] = value.get<std::string>();
    }
}

// CopyFrom copies from another HyperValues
void params::HyperValues::CopyFrom(HyperValues &cp) {
    for (const auto& [key, value] : cp.map) {
        map[key] = value;
    }
}

// ParamByName returns given parameter by name (just does the map access)
// Returns "" if not found -- use Try version for error
params::HyperValues params::Hypers::ParamByName(std::string name) {
    if (map.find(name) == map.end()){
        std::string err = "params::Params: parameter named " + name + " not found";
        throw std::invalid_argument(err);
    }
    else {
        return map[name];
    }
}

// SetByName sets given parameter by name to given value.
// (just a wrapper around map set function)
void params::Hypers::SetByName(std::string name, HyperValues& value) {
    map[name] = value;
}

// CopyFrom copies hyper vals from source
void params::Hypers::CopyFrom(Hypers& cp) {
    for (const auto& [key, value] : cp.map) {
        if (map.find(key) == map.end()){ // if key is new
            map[key] = value;
        }
        else { // if key exists, copy to underlying HyperValues object
            params::HyperValues hpv = value;
            map[key].CopyFrom(hpv);
        }
    }
}

// DeleteValOnly deletes entries that only have a "Val" entry.
// This happens when applying a param Sheet using Flex params
// to compile values using styling logic
void params::Hypers::DeleteValOnly(){
    for (const auto& [key, value] : map) {
        if (value.map.size()==1 && value.map.count("Val")){
            map.erase(key);
        }
    }
}

// TargetType returns the first part of the path, indicating what type of
// object the params apply to.  Uses the first item in the map (which is random)
// everything in the map must have the same target.
std::string params::Hypers::TargetType() {
    for (auto &[pt, value]: map) {
        return strings::split(pt, '.')[0];
    }
    return "";
}

// Path returns the second part of the path after the target type,
// indicating the path to the specific parameter being set.
std::string params::Hypers::Path(std::string path) {
    std::vector<std::string> s = strings::split(path, ','); //copy
    s.erase(s.begin()); //drop first element
    return strings::join(s, ".");
}

std::string params::Hypers::Apply(std::any obj, bool setMsg) {
    std::string objNm = "";
    try {
        params::StylerObject* sty = std::any_cast<params::StylerObject*>(obj);
        std::string objNm = sty->StyleName();
    } catch (const std::bad_any_cast& e) {
    }

    try {
        params::Hypers* hv = std::any_cast<params::Hypers*>(obj);

        hv->CopyFrom(*this);
        return "";
        
    }
    catch (const std::bad_any_cast& e) {
        std::vector<std::string> errs;

        for (auto& [pt, v] : map) {
            std::string path = Path(pt);
            std::string val;
            if (v.map.count("Val") == 0) {
                continue;
            } else {
                val = v.map["Val"];
            }
            std::string err = SetParam(obj, path, val);
            if (err == "") {
                if (setMsg) {
                    std::string errStr = objNm + " Set param path: " + pt +
                        " to value: " + val;
                    std::cout << errStr << std::endl;
                }
            } else {
                errs.push_back(err);
            }
        }
        return strings::join(errs, "");
    }
}

void params::to_json(json &j, const params::Hypers &flx) {
    for (const auto& [key, value] : flx.map) {
        j[key] = json(value.JSONString());
    }
}

void params::Sel::SetFloat(std::string param, float val) {
    this->ParamsSet.SetByName(param, std::to_string(val));
}

void params::Sel::SetString(std::string param, std::string val) {
    this->ParamsSet.SetByName(param, val);
}

std::string params::Sel::ParamValue(std::string param) {
    return ParamsSet.ParamByName(param);
}

// Apply checks if Sel selector applies to this object according to (.Class, #Name, Type)
// using the params.Styler interface, and returns false if it does not.
// The TargetType of the Params is always tested against the obj's type name first.
// If it does apply, or is not a Styler, then the Params values are set.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// It always prints a message if a parameter fails to be set, and returns an error.
bool params::Sel::Apply(std::any obj, bool setMsg) {
    if (!TargetTypeMatch(obj) || !SelMatch(obj)) {
		return false;//, nil
	}

	std::string errp = ParamsSet.Apply(obj, setMsg);
	std::string errh = HyperSet.Apply(obj, setMsg);
	if (errp != "" && errh != "") {
		return false;//, errp
	} else {
        return true;
    }
	// return true;//, errp
}

// TargetTypeMatch return true if target type applies to object
bool params::Sel::TargetTypeMatch(std::any obj) {
    std::string trg = ParamsSet.TargetType();
    std::string trgh = HyperSet.TargetType();
    try {
        StylerObject *styler = std::any_cast<params::StylerObject*>(obj);
        std::string tnm = styler->StyleType();
        if (tnm == trg) {
            return true;
        } else if (tnm == trgh){
            return true;
        }
    } catch (const std::bad_any_cast& e) {

    }
    return false;

	// TODO: Figure out what the lines below should be doing...
	// tnm := reflectx.NonPointerType(reflect.TypeOf(obj)).Name()
	// return tnm == trg || tnm == trgh
}

// SelMatch returns true if Sel selector matches the target object properties
bool params::Sel::SelMatch(std::any obj) {
    try {
        StylerObject *styler = std::any_cast<params::StylerObject*>(obj);
        // std::string gotyp = styler->Name();
        return params::SelMatch(Sel, styler->StyleName(), styler->StyleClass(), styler->StyleType());//, gotyp);
    } catch (const std::bad_any_cast& e) {
        return true;
    }
    // styler, has := obj.(Styler)
	// if !has {
	// 	return true // default match if no styler..
	// }
	// if styob, has := obj.(StylerObject); has {
	// 	obj = styob.StyleObject()
	// }
	// gotyp := reflectx.NonPointerType(reflect.TypeOf(obj)).Name()
	// return SelMatch(ps.Sel, styler.StyleName(), styler.StyleClass(), styler.StyleType(), gotyp)
}

// void params::FlexVal::CopyFrom(params::FlexVal cp) {
//     Nm = cp.Nm;
//     Type = cp.Type;
//     Cls = cp.Cls;
//     if (Obj.type() == typeid(params::Hypers) && cp.Obj.type() == typeid(params::Hypers)) { // TODO CHECK IF THIS WORKS
//         std::any_cast<params::Hypers>(Obj).CopyFrom(std::any_cast<params::Hypers&>(cp.Obj));
//     }

// }

// void params::to_json(json &j, const params::FlexVal &flx) {
//     j["Nm"]= flx.Nm; //does this turn it into json, or from json?
//     j["Type"]= flx.Type;
//     j["Cls"]= flx.Cls;

//     if (flx.Obj.type() == typeid(params::Hypers)) {
//         params::Hypers hyp = std::any_cast<params::Hypers>(flx.Obj);
//         json substr = hyp;
//         j["Obj"]= substr;
//     }
//     else {
//         std::string objStr = "\"" + anyToString(flx.Obj)  + "\"";
//         j["Obj"]= objStr;
//     }
// }

// TODO complete this for any types that might appear
// bool IsStruct(std::any &obj){
//     std::string typ = obj.type().name();
//     if (typ == typeid(std::map<std::string,std::string>).name()){
//         return false;
//     }
//     else {
//         return true;
//     }
    // if (typ == typeid(params::Flex).name()){ // check for all acceptable types
    //     return true;
    // }
    // else if (typ == typeid(params::Params).name()){
    //     return true;
    // }
    // else {
    //     return false;
    // }
// }

// FindParam parses the path and recursively tries to find the parameter pointed to
// by the path (dot-delimited field names).
// Returns error if not found, and always also emits error messages --
// the target type should already have been identified and this should only
// be called when there is an expectation of the path working.
// TODO MAYBE JSON OBJECTS CAN BE CONVERTED TO PARAMETER STRUCTS RATHER THAN REFLECTION
// TODO TAKE A CLOSER LOOK AT THIS SYNTAX AND REWRITE IT
// TODO Add code to each struct to map from string val/path to struct members
std::any params::FindParam(std::any val, std::string path) {
    StylerObject* valptr = std::any_cast<StylerObject*>(val);
    StylerObject &npv = *valptr;
    if (valptr == nullptr) {
        throw std::invalid_argument("params.FindParam: val given is nullptr! path: " + path + "\n");
        return val;
    }
	std::vector<std::string> paths = strings::split(path, '.');
	std::string fnm = paths[0];
	if (npv.ParamNameMap.count(fnm)==0) {
		std::string err = "params.FindParam: could not find Field named: "+ fnm +" in struct: "+ npv.StyleName() +" path: "+ path +"\n";
        std::cerr << err;
		return val;
	}

    std::any newVal = npv.ParamNameMap[fnm];
	if (paths.size() == 1) {
		return newVal;
	}
    std::vector<std::string> newPaths = paths;
    newPaths.erase(newPaths.begin());
	return FindParam(newVal, strings::join(newPaths, "."));
}

// SetParam sets parameter at given path on given object to given value
// converts the string param val as appropriate for target type.
// returns error if path not found or cannot set (always logged).
std::string params::SetParam(std::any obj, std::string path, std::string val) {
    // npv := reflectx.NonPointerValue(reflect.ValueOf(obj))
	// if npv.Kind() == reflect.Map { // only for string maps
	// 	npv.SetMapIndex(reflect.ValueOf(path), reflect.ValueOf(val))
	// 	return nil
	// }
    StylerObject &parentObj = *std::any_cast<StylerObject*>(obj);
    return parentObj.SetByPath(path, val);
}

// GetParam gets parameter value at given path on given object.
// converts target type to float64.
// returns error if path not found or target is not a numeric type (always logged).
float params::GetParam(std::any obj, std::string path) {
    StylerObject &styler = *std::any_cast<StylerObject*>(obj);
    return styler.GetByPath(path);
}

// SelMatch returns true if Sel selector matches the target object properties
bool params::SelMatch(std::string sel, std::string name, std::string cls, std::string styp) {
    if (sel == "") {
		return false;
	}
	if (sel[0] == '.') { // class
		return params::ClassMatch(sel.substr(1), cls);
	}
	if (sel[0] == '#') { // name
		return name == sel.substr(1);
	}
	return styp == sel; // type
}

// ClassMatch returns true if given class names.
// handles space-separated multiple class names
bool params::ClassMatch(std::string sel, std::string cls) {
    std::vector<std::string> clss = strings::split(cls, ' ');
	for (std::string &cl: clss) {
		if (strings::TrimSpace(cl) == sel) {
			return true;
		}
	}
	return false;
}



// // Init initializes the Flex map with given set of flex values.
// void params::Flex::Init(std::vector<params::FlexVal> vals) {
//     for (FlexVal& val: vals){
//         map[val.Nm] = val;
//     }
// }

// // ApplySheet applies given sheet of parameters to each element in Flex
// void params::Flex::ApplySheet(params::Sheet *sheet, bool setMsg) {
//     for (const auto& [key, value]: this->map){
//         std::any obj = value;
//         sheet->Apply(obj, setMsg);
//     }
// }

// // CopyFrom copies hyper vals from source
// void params::Flex::CopyFrom(Flex cp) {
//     for (const auto& [key, value]: cp.map) {
//         if (map.count(key) == 0){
//             map[key] = params::FlexVal();
//         }
//         map[key].CopyFrom(cp.map[key]);
//     }
// }

// // JSONString returns a string representation of Flex params
// std::string params::Flex::JSONString() {
//     // Create a new JSON object from the map
//     json reconstructedJson;
//     for (const auto& [key, value] : map) {
//         reconstructedJson[key] = json(value);
//     }
//     // Convert to a JSON string
//     std::string jsonStr = reconstructedJson.dump();
//     return jsonStr;
// }

// ElemLabel satisfies the core.SliceLabeler interface to provide labels for slice elements
std::string params::Sheet::ElemLabel(int idx){
    return sel[idx].Sel;
}

// SelByNameTry returns given selector within the Sheet, by Name.
// Returns nil and error if not found.
// params::Sel* params::Sheet::SelByNameTry(std::string sel) {
//     params::Sel* sl = SelByName(sel);
//     if (sl == nullptr){
//         std::string err = "params::Sheet Sel named " + sel + " not found";
//         throw std::invalid_argument(err);
//     }
//     else {
//         return sl;
//     }
// }

// SelByName returns given selector within the Sheet, by Name.
// Returns nil if not found -- use Try version for error
params::Sel* params::Sheet::SelByName(std::string sel) {
    for (params::Sel &slref: this->sel){
        params::Sel *sl = &slref;
        if (sl->Sel== sel){
            return sl;
        }
    }
    return nullptr;
}

// SetFloat sets the value of given parameter, in selection sel
void params::Sheet::SetFloat(std::string sel, std::string param, float val) {
    params::Sel* sl = SelByName(sel);
    sl->SetFloat(param, val);
}

// SetString sets the value of given parameter, in selection sel
void params::Sheet::SetString(std::string sel, std::string param, std::string val) {
    params::Sel* sl = SelByName(sel);
    sl->SetString(param, val);
}

// ParamVal returns the value of given parameter, in selection sel
std::string params::Sheet::ParamValue(std::string sel, std::string param) {
    params::Sel* sl = SelByName(sel);
    return sl->ParamValue(param);
}

// Apply applies entire sheet to given object, using param.Sel's in order
// see param.Sel.Apply() for details.
// returns true if any Sel's applied, and error if any errors.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// It always prints a message if a parameter fails to be set, and returns an error.
bool params::Sheet::Apply(std::any obj, bool setMsg) {
    bool applied = false;
	std::vector<std::string> errs;
	for (Sel &slref: sel) {
        Sel *sl = &slref;
		bool app = sl->Apply(obj, setMsg);
		if (app) {
			applied = true;
			sl->NMatch++;
			// if hist, ok := obj.(History); ok {
			// 	hist.ParamsApplied(sl)
			// }
		}
	}
	return applied;
}

// SelMatchReset resets the Sel.NMatch counter used to find cases where no Sel
// matched any target objects.  Call at start of application process, which
// may be at an outer-loop of Apply calls (e.g., for a Network, Apply is called
// for each Layer and Path), so this must be called separately.
// See SelNoMatchWarn for warning call at end.
void params::Sheet::SelMatchReset(std::string setName) {
    for (Sel &slref: sel) {
        Sel *sl = &slref;
		sl->NMatch = 0;
		sl->SetName = setName;
	}
}

// SelNoMatchWarn issues warning messages for any Sel selectors that had no
// matches during the last Apply process -- see SelMatchReset.
// The setName and objName provide info about the Set and obj being applied.
// Returns an error message with the non-matching sets if any, else nil.
void params::Sheet::SelNoMatchWarn(std::string setName, std::string objName) {
    std::string msg = "";
	for (Sel &slref: sel) {
        Sel *sl = &slref;
		if (sl->NMatch == 0) {
			msg += "\tSel: " + sl->Sel + "\n";
		}
	}
	if (msg != "") {
		msg = "param.Sheet from Set: "+ setName +" for object: " + objName + " had the following non-matching Selectors:\n" + msg;
		std::cerr << msg;
	}
}

// SheetByNameTry tries to find given sheet by name, and returns error
// if not found (also logs the error)
// params::Sheet *params::Set::SheetByNameTry(std::string name) {
//     for (const auto& [key, value]: SheetSet){
//         if (key == name){
//             return &SheetSet[name];;
//         }
//     }
//     std::string err = "params::Sheets sheet named " + name + " not found";
//     throw std::invalid_argument(err);
// }

// SheetByName finds given sheet by name -- returns nil if not found.
// Use this when sure the sheet exists -- otherwise use Try version.
params::Sheet *params::Sets::SheetByName(std::string name) {
    return &(sheets[name]);
}

// ValidateSheets ensures that the sheet names are among those listed -- returns
// error message for any that are not. Helps catch typos and makes sure params are
// applied properly. Automatically logs errors.
// void params::Sets::ValidateSheets(std::vector<std::string> valids) {
//     std::vector<std::string> invalids;
    
//     for (const auto& [key, value]: map){
//         bool got = false;
//         for (const std::string &vl: valids){
//             if (key == vl) {
//                 got = true;
//             }
//         }
//         if (!got){
//             invalids.push_back(key);
//         }
//     }
//     if (invalids.size() > 0){
//         std::ostringstream oss;
//         oss << "params.Set: Invalid sheet names: \n";
//         for (const auto& name : invalids) {
//             oss << "\t" << name << ",\n";
//         }
//         std::string err = oss.str();
//         throw std::invalid_argument(err);
//     }
// }

// SetFloat sets the value of given parameter, in selection sel,
// in sheet
void params::Sets::SetFloat(std::string sheet, std::string sel, std::string param, float val) {
    params::Sheet *sp= SheetByName(sheet);
    sp->SetFloat(sel, param, val);
}

// SetString sets the value of given parameter, in selection sel,
// in sheet
void params::Sets::SetString(std::string sheet, std::string sel, std::string param, std::string val) {
    params::Sheet *sp= SheetByName(sheet);
    sp->SetString(sel, param, val);
}

// ParamVal returns the value of given parameter, in selection sel,
// in sheet
std::string params::Sets::ParamValue(std::string sheet, std::string sel, std::string param) {
    params::Sheet *sp= SheetByName(sheet);
    return sp->ParamValue(sel, param);
}

// params::StylerObject::StylerObject() {
//     InitParamMaps();
// }

std::string params::StylerObject::SetByName(std::string varName, std::string value) {
    std::string err = "";
    void *varPtr = ParamNameMap[varName];
    if (ParamNameMap.count(varName) == 0) {
        err = "Error: variable named " + varName + " not found.";
        return err;
    }
    if (ParamTypeMap[varName] == &typeid(int)) {
        int val = std::stoi(value);
        int *ptr = (int *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == &typeid(float)) {
        float val = std::stof(value);
        float *ptr = (float *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == &typeid(bool)) {
        bool val;
        std::istringstream(value) >> std::boolalpha >> val;
        bool *ptr = (bool *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == &typeid(std::vector<int>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = strings::split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<int> *vectorPtr = (std::vector<int> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            vectorPtr->push_back(std::stoi(item)); // TODO figure out how to handle 
        }
    } else if (ParamTypeMap[varName] == &typeid(std::vector<float>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = strings::split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<float> *vectorPtr = (std::vector<float> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            vectorPtr->push_back(std::stof(item)); // TODO figure out how to handle 
        }
    } else if (ParamTypeMap[varName] == &typeid(std::vector<bool>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = strings::split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<bool> *vectorPtr = (std::vector<bool> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            bool val;
            std::istringstream(item) >> std::boolalpha >> val;
            vectorPtr->push_back(val); // TODO figure out how to handle 
        }
    // } else if (ParamTypeMap[varName] == &typeid(params::StylerObject)) {
        // TODO HANDLE THE CASE OF NESTED PARAMS?
        // ...maybe do nothing...
    } else {
        // if (StylerObject *obj = dynamic_cast<StylerObject*>(ParamNameMap[varName])) {
        //     // TODO HANDLE THE CASE OF NESTED PARAMS?
        // // ...maybe do nothing...
        // }
        err = "Error: type of variable named " + varName + " not handled.";
    }
    return err;
}

float params::StylerObject::GetByName(std::string varName) {
    float var;
    void *varPtr = ParamNameMap[varName];
    if (ParamNameMap.count(varName) == 0) {
        throw std::invalid_argument("Error: variable named " + varName + " not found.");
    }
    if (ParamTypeMap[varName] == &typeid(int)) {
        // int val = std::stoi(value);
        int *ptr = (int *)varPtr;
        var = *ptr;
    } else if (ParamTypeMap[varName] == &typeid(float)) {
        // float val = std::stof(value);
        float *ptr = (float *)varPtr;
        // *ptr = val;
        var = *ptr;
    } else if (ParamTypeMap[varName] == &typeid(bool)) {
        // bool val;
        // std::istringstream(value) >> std::boolalpha >> val;
        bool *ptr = (bool *)varPtr;
        // *ptr = val;
        var = *ptr;
    // } else if (ParamTypeMap[varName] == &typeid(params::StylerObject)) {
        // TODO HANDLE THE CASE OF NESTED PARAMS?
        // ...maybe do nothing...
    } else {
        // if (StylerObject *obj = dynamic_cast<StylerObject*>(ParamNameMap[varName])) {
        //     // TODO HANDLE THE CASE OF NESTED PARAMS?
        // // ...maybe do nothing...
        // }
        throw std::invalid_argument("Error: type of variable named " + varName + " not handled. Type info: " + ParamTypeMap[varName]->name());
    }
    return var;
}

std::string params::StylerObject::SetByPath(std::string path, std::string value) {
    std::vector<std::string> paths = strings::split(path, '.');
    std::string &name = paths[0];
    if (paths.size() == 1) {
        return SetByName(name, value);
    } else {
        StylerObject *child = (StylerObject*)ParamNameMap[paths[0]];
        paths.erase(paths.begin());
        return child->SetByPath(strings::join(paths,"."), value);
    }
}

float params::StylerObject::GetByPath(std::string path) {
    std::vector<std::string> paths = strings::split(path, '.');
    std::string &name = paths[0];
    if (paths.size() == 1) {
        return GetByName(name);
    } else {
        StylerObject *child = (StylerObject*)ParamNameMap[paths[0]];
        paths.erase(paths.begin());
        return child->GetByPath(strings::join(paths,"."));
    }
}

void *params::StylerObject::GetStyleObject() {
    return (void *)this;
}
