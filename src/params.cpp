#include "params.hpp"
#include <stdexcept>
#include <typeinfo>
#include <iostream>

// Template for supporting any to string conversion
template <typename T>
std::string anyToString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


std::string params::Params::ParamByNameTry(std::string name) {
    if (map.find(name) == map.end()){
        std::string err = "params::Params: parameter named " + name + " not found";
        throw std::invalid_argument(err);
    }
    else {
        return map[name];
    }
}

std::string params::Params::ParamByName(std::string name) {
    return map[name];
}

void params::Params::SetByName(std::string name, std::string value) {
    map[name] = value;
}

// No split operation in C++, so...
std::vector<std::string> split(const std::string &str, const char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
    std::string result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result += tokens[i];
        if (i != tokens.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

// Path returns the second part of the path after the target type,
// indicating the path to the specific parameter being set.
// e.g., Layer.Acts.Kir.Gbar -> Acts.Kir.Gbar
std::string params::Params::Path(std::string path) {
    std::vector<std::string> s = split(path, ',');
    s.erase(s.begin()); //drop first element
    return join(s, ".");
}

// TargetType returns the first part of the path, indicating what type of
// object the params apply to.  Uses the first item in the map (which is random)
// everything in the map must have the same target.
std::string params::Params::TargetType() {
    for (const auto& [key, value] : map) {
        return split(key, '.')[0];
    }
}

// Apply applies all parameter values to given object.
// Object must already be the appropriate target type based on
// the first element of the path (see TargetType method).
// If setMsg is true, then it will log a confirmation that the parameter
// was set (it always prints an error message if it fails to set the
// parameter at given path, and returns error if so).
bool  params::Params::Apply(std::any & obj, bool setMsg) {
    params::StylerObject& sty = std::any_cast<params::StylerObject&>(obj);
    std::string objNm = sty.StyleName();

    std::vector<std::string> errs;

    for (const auto& [key, value] : map) {
        std::string path = Path(key);

        try {
            params::Hypers& hv = std::any_cast<params::Hypers&>(obj);
            if (hv.map.count(path)){
                hv.map[path].map["Val"] = value;
            }
            else {
                hv.map[path] = params::HyperValues{map: {{"Val", value}}};
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
    // return join(errs, "");
    return true; // TODO: REDO this function to correctly identify if it was set or not
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

// ParamByNameTry returns given parameter, by name.
// Returns error if not found.
params::HyperValues params::Hypers::ParamByNameTry(std::string name) {
    if (map.find(name) == map.end()){
        std::string err = "params::Params: parameter named " + name + " not found";
        throw std::invalid_argument(err);
    }
    else {
        return map[name];
    }
}

// ParamByName returns given parameter by name (just does the map access)
// Returns "" if not found -- use Try version for error
params::HyperValues params::Hypers::ParamByName(std::string name) {
    return map[name];
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

void params::to_json(json &j, const params::Hypers &flx) {
    for (const auto& [key, value] : flx.map) {
        j[key] = json(value.JSONString());
    }
}

// void params::Sel::SetFloat(std::string param, float val) {
//     this->Params.SetByName(param, std::to_string(val));
// }

// void params::Sel::SetString(std::string param, std::string val) {
//     this->Params.SetByName(param, val);
// }

// std::string params::Sel::ParamValue(std::string param) {
//     return Params.ParamByNameTry(param);
// }

void params::FlexVal::CopyFrom(params::FlexVal cp) {
    Nm = cp.Nm;
    Type = cp.Type;
    Cls = cp.Cls;
    if (Obj.type() == typeid(params::Hypers) && cp.Obj.type() == typeid(params::Hypers)) { // TODO CHECK IF THIS WORKS
        std::any_cast<params::Hypers>(Obj).CopyFrom(std::any_cast<params::Hypers&>(cp.Obj));
    }

}

void params::to_json(json &j, const params::FlexVal &flx) {
    j["Nm"]= flx.Nm; //does this turn it into json, or from json?
    j["Type"]= flx.Type;
    j["Cls"]= flx.Cls;

    if (flx.Obj.type() == typeid(params::Hypers)) {
        params::Hypers hyp = std::any_cast<params::Hypers>(flx.Obj);
        json substr = hyp;
        j["Obj"]= substr;
    }
    else {
        std::string objStr = "\"" + anyToString(flx.Obj)  + "\"";
        j["Obj"]= objStr;
    }
}

// TODO complete this for any types that might appear
bool IsStruct(std::any &obj){
    std::string typ = obj.type().name();
    if (typ == typeid(std::map<std::string,std::string>).name()){
        return false;
    }
    else {
        return true;
    }
    // if (typ == typeid(params::Flex).name()){ // check for all acceptable types
    //     return true;
    // }
    // else if (typ == typeid(params::Params).name()){
    //     return true;
    // }
    // else {
    //     return false;
    // }
}

// FindParam parses the path and recursively tries to find the parameter pointed to
// by the path (dot-delimited field names).
// Returns error if not found, and always also emits error messages --
// the target type should already have been identified and this should only
// be called when there is an expectation of the path working.
// TODO MAYBE JSON OBJECTS CAN BE CONVERTED TO PARAMETER STRUCTS RATHER THAN REFLECTION
// TODO TAKE A CLOSER LOOK AT THIS SYNTAX AND REWRITE IT
// TODO Add code to each struct to map from string val/path to struct members
std::any params::FindParam(std::any &val, std::string path) {
    if (IsStruct(val)){
        std::string err = "TODO FIX FINDPARAM... idk how\n\t";
        err += "type of val was: " + std::string(val.type().name()) + "\n";
        throw std::invalid_argument(err);
    }
    else {
        std::map<std::string,std::string>& map = std::any_cast<std::map<std::string,std::string>&>(val);
        std::vector<std::string> pathParts = split(path, '.');
        std::string firstName = pathParts[0];
        pathParts.erase(pathParts.begin());
        if (map.count(firstName)==0) { // key does not exist
            std::string err = "params.FindParam: could not find Field named: " + firstName;
                err += " in the 'any' container\n";  
            throw std::invalid_argument(err);
        }
        else{
            std::any obj =  &map[firstName];
            if (pathParts.size() == 1){
                return obj;
            }
            else{
                return FindParam(obj, join(pathParts, "."));
            }
        }
    }
}

// SetParam sets parameter at given path on given object to given value
// converts the string param val as appropriate for target type.
// returns error if path not found or cannot set (always logged).
std::string params::SetParam(std::any &obj, std::string path, std::string val) {
    if (obj.type() == typeid(std::map<std::string,std::string>)) {
        auto& map = std::any_cast<std::map<std::string,std::string>&>(obj);
        map[path] = val;
        return ""; // No error
    }
    else { // The obj is probably a struct, but how to support each struct?

        return "SetParam ERROR: Type of obj is: " + std::string(obj.type().name()) + "\n";
    }
}

float params::GetParam(std::any &obj, std::string path) {
    return 0.0;
}

// Init initializes the Flex map with given set of flex values.
void params::Flex::Init(std::vector<params::FlexVal> vals) {
    for (FlexVal& val: vals){
        map[val.Nm] = val;
    }
}

// ApplySheet applies given sheet of parameters to each element in Flex
void params::Flex::ApplySheet(params::Sheet *sheet, bool setMsg) {
    for (const auto& [key, value]: this->map){
        std::any obj = value;
        sheet->Apply(obj, setMsg);
    }
}

// CopyFrom copies hyper vals from source
void params::Flex::CopyFrom(Flex cp) {
    for (const auto& [key, value]: cp.map) {
        if (map.count(key) == 0){
            map[key] = params::FlexVal();
        }
        map[key].CopyFrom(cp.map[key]);
    }
}

// JSONString returns a string representation of Flex params
std::string params::Flex::JSONString() {
    // Create a new JSON object from the map
    json reconstructedJson;
    for (const auto& [key, value] : map) {
        reconstructedJson[key] = json(value);
    }
    // Convert to a JSON string
    std::string jsonStr = reconstructedJson.dump();
    return jsonStr;
}

// ElemLabel satisfies the core.SliceLabeler interface to provide labels for slice elements
std::string params::Sheet::ElemLabel(int idx){
    return sel[idx].Sel;
}

// SelByNameTry returns given selector within the Sheet, by Name.
// Returns nil and error if not found.
params::Sel* params::Sheet::SelByNameTry(std::string sel) {
    params::Sel* sl = SelByName(sel);
    if (sl == nullptr){
        std::string err = "params::Sheet Sel named " + sel + " not found";
        throw std::invalid_argument(err);
    }
    else {
        return sl;
    }
}

// SelByName returns given selector within the Sheet, by Name.
// Returns nil if not found -- use Try version for error
params::Sel* params::Sheet::SelByName(std::string sel) {
    for (params::Sel &sl: this->sel){
        if (sl.Sel== sel){
            return &sl;
        }
    }
    return nullptr;
}

// SetFloat sets the value of given parameter, in selection sel
void params::Sheet::SetFloat(std::string sel, std::string param, float val) {
    params::Sel* sl = SelByNameTry(sel);
    sl->SetFloat(param, val);
}

// SetString sets the value of given parameter, in selection sel
void params::Sheet::SetString(std::string sel, std::string param, std::string val) {
    params::Sel* sl = SelByNameTry(sel);
    sl->SetString(param, val);
}

// ParamVal returns the value of given parameter, in selection sel
std::string params::Sheet::ParamVal(std::string sel, std::string param) {
    params::Sel* sl = SelByNameTry(sel);
    return sl->ParamValue(param);
}

// SheetByNameTry tries to find given sheet by name, and returns error
// if not found (also logs the error)
params::Sheet *params::Set::SheetByNameTry(std::string name) {
    for (const auto& [key, value]: SheetSet){
        if (key == name){
            return &SheetSet[name];;
        }
    }
    std::string err = "params::Sheets sheet named " + name + " not found";
    throw std::invalid_argument(err);
}

// SheetByName finds given sheet by name -- returns nil if not found.
// Use this when sure the sheet exists -- otherwise use Try version.
params::Sheet *params::Set::SheetByName(std::string name) {
    return &SheetSet[name];
}

// ValidateSheets ensures that the sheet names are among those listed -- returns
// error message for any that are not. Helps catch typos and makes sure params are
// applied properly. Automatically logs errors.
void params::Set::ValidateSheets(std::vector<std::string> valids) {
    std::vector<std::string> invalids;
    
    for (const auto& [key, value]: SheetSet){
        bool got = false;
        for (const std::string &vl: valids){
            if (key == vl) {
                got = true;
            }
        }
        if (!got){
            invalids.push_back(key);
        }
    }
    if (invalids.size() > 0){
        std::ostringstream oss;
        oss << "params.Set: Invalid sheet names: \n";
        for (const auto& name : invalids) {
            oss << "\t" << name << ",\n";
        }
        std::string err = oss.str();
        throw std::invalid_argument(err);
    }
}

// SetFloat sets the value of given parameter, in selection sel,
// in sheet
void params::Set::SetFloat(std::string sheet, std::string sel, std::string param, float val) {
    params::Sheet *sp= SheetByNameTry(sheet);
    sp->SetFloat(sel, param, val);
}

// SetString sets the value of given parameter, in selection sel,
// in sheet
void params::Set::SetString(std::string sheet, std::string sel, std::string param, std::string val) {
    params::Sheet *sp= SheetByNameTry(sheet);
    sp->SetString(sel, param, val);
}

// ParamVal returns the value of given parameter, in selection sel,
// in sheet
std::string params::Set::ParamValue(std::string sheet, std::string sel, std::string param) {
    params::Sheet *sp= SheetByNameTry(sheet);
    return sp->ParamVal(sel, param);
}

void params::StylerObject::SetByName(std::string varName, std::string value) {
    void *varPtr = ParamNameMap[varName];
    if (ParamNameMap.count(varName) == 0) throw std::runtime_error("Error: variable named " + varName + " not found.");
    if (ParamTypeMap[varName] == typeid(int)) {
        int val = std::stoi(value);
        int *ptr = (int *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == typeid(float)) {
        float val = std::stof(value);
        float *ptr = (float *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == typeid(bool)) {
        bool val;
        std::istringstream(value) >> std::boolalpha >> val;
        bool *ptr = (bool *)varPtr;
        *ptr = val;
    } else if (ParamTypeMap[varName] == typeid(std::vector<int>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<int> *vectorPtr = (std::vector<int> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            vectorPtr->push_back(std::stoi(item)); // TODO figure out how to handle 
        }
    } else if (ParamTypeMap[varName] == typeid(std::vector<float>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<float> *vectorPtr = (std::vector<float> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            vectorPtr->push_back(std::stof(item)); // TODO figure out how to handle 
        }
    } else if (ParamTypeMap[varName] == typeid(std::vector<bool>)) {
        value.pop_back(); // get rid of ']' character
        value.erase(value.begin()); // get rid of '[' character
        std::vector<std::string> vectorString = split(value, ','); //TODO figure out how to handle optional spaces

        std::vector<bool> *vectorPtr = (std::vector<bool> *) varPtr;
        vectorPtr->reserve(vectorString.size());
        for (std::string item: vectorString){
            bool val;
            std::istringstream(item) >> std::boolalpha >> val;
            vectorPtr->push_back(val); // TODO figure out how to handle 
        }
    } else if (ParamTypeMap[varName] == typeid(params::StylerObject)) {
        // TODO HANDLE THE CASE OF NESTED PARAMS?
        // ...maybe do nothing...
    } else {
        throw std::runtime_error("Error: type of variable named " + varName + " not handled.");
    }
}
