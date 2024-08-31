#pragma once
#include <string>
#include <map>
#include <any>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace params {

    // The params.Styler interface exposes TypeName, Class, and Name methods
    // that allow the params.Sel CSS-style selection specifier to determine
    // whether a given parameter applies.
    // Adding Set versions of Name and Class methods is a good idea but not
    // needed for this interface, so they are not included here.
    struct StylerObject{
        std::map<std::string, void*> ParamNameMap;
        std::map<std::string, std::type_info> ParamTypeMap;

        StylerObject(){InitParamMaps();};

        // StyleType returns the name of this type for CSS-style matching.
        // This is used for CSS Sel selector with no prefix.
        // This type is used *in addition* to the actual Go type name
        // of the object, and is a kind of type-category (e.g., Layer
        // or Path in emergent network objects).
        virtual std::string StyleType();

        // StyleClass returns the space-separated list of class selectors (tags).
        // Parameters with a . prefix target class tags.
        // Do NOT include the . in the Class tags on Styler objects;
        // The . is only used in the Sel selector on the params.Sel.
        virtual std::string StyleClass();

        // StyleName returns the name of this object.
        // Parameters with a # prefix target object names, which are typically
        // unique.  Note, do not include the # prefix in the actual object name,
        // only in the Sel selector on params.Sel.
        virtual std::string StyleName();

        // InitParamMaps is overridden for each object and initializes the param maps
        // that allow the params objects to access and modify params by string name
        virtual void InitParamMaps();

        // Finds the appropriate parameter to set from the string given
        void SetByName(std::string varName, std::string value);
    };


    // Params is a name-value map for parameter values that can be applied
    // to any numeric type in any object.
    // The name must be a dot-separated path to a specific parameter, e.g., Path.Learn.Lrate
    // The first part of the path is the overall target object type, e.g., "Path" or "Layer",
    // which is used for determining if the parameter applies to a given object type.
    //
    // All of the params in one map must apply to the same target type because
    // only the first item in the map (which could be any due to order randomization)
    // is used for checking the type of the target.  Also, they all fall within the same
    // Sel selector scope which is used to determine what specific objects to apply the
    // parameters to.
    struct Params {
        std::map<std::string, std::string> map;
        std::string ParamByNameTry(std::string name);
        std::string ParamByName(std::string name);
        void SetByName(std::string name, std::string value);
        std::string Path(std::string path);
        std::string TargetType();
        bool Apply(std::any &obj, bool setMsg);
    };

    // HyperValues is a string-value map for storing hyperparameter values
    struct HyperValues {
        std::map<std::string, std::string> map;

        std::string JSONString() const;
        void SetJSONString(std::string str);
        void CopyFrom(HyperValues& cp);
    };
    
    // Hypers is a parallel structure to Params which stores information relevant
    // to hyperparameter search as well as the values.
    // Use the key "Val" for the default value. This is equivalant to the value in
    // Params. "Min" and "Max" guid the range, and "Sigma" describes a Gaussian.
    struct Hypers {
        std::map<std::string, HyperValues> map;

        HyperValues ParamByNameTry(std::string name);
        HyperValues ParamByName(std::string name);
        void SetByName(std::string name, HyperValues& value);
        void CopyFrom(Hypers& cp);
        void DeleteValOnly();
        // std::string JSONString();

        void Apply(std::any &obj, bool setMsg);
    };
    void to_json(json& j, const Hypers& flx);
    // void from_json(json& j, const Hypers& flx);
    
    // typedef std::map<std::string, HyperValues> Hypers;
    
    
    // params.Sel specifies a selector for the scope of application of a set of
    // parameters, using standard css selector syntax (. prefix = class, # prefix = name,
    // and no prefix = type)
    struct Sel {
        
        std::string Sel, // selector for what to apply the parameters to, using standard css selector syntax: .Example applies to anything with a Class tag of 'Example', #Example applies to anything with a Name of 'Example', and Example with no prefix applies to anything of type 'Example'
            Desc; // description of these parameter values -- what effect do they have?  what range was explored?  it is valuable to record this information as you explore the params.
        Params ParamsSet; // parameter values to apply to whatever matches the selector
        Hypers HyperSet; // Put your hyperparams here
        int NMatch; // number of times this selector matched a target during the last Apply process -- a warning is issued for any that remain at 0 -- see Sheet SelMatchReset and SelNoMatchWarn methods
        std::string SetName; // name of current Set being applied

        void SetFloat(std::string param, float val);
        void SetString(std::string param, std::string val);
        std::string ParamValue(std::string param);

        void Apply(std::any &obj, bool setMsg);
    };
    
    

    // Sheet is a CSS-like style-sheet of params.Sel values, each of which represents
    // a different set of specific parameter values applied according to the Sel selector:
    // .Class #Name or Type.
    //
    // The order of elements in the Sheet list is critical, as they are applied
    // in the order given by the list (slice), and thus later Sel's can override
    // those applied earlier.  Thus, you generally want to have more general Type-level
    // parameters listed first, and then subsequently more specific ones (.Class and #Name)
    //
    // This is the highest level of params that has an Apply method -- above this level
    // application must be done under explicit program control.
    // typedef Sel** Sheet; // Array of Sel*
    struct Sheet {
        std::vector<Sel> sel;
        Sheet(){sel = std::vector<Sel>();};

        std::string ElemLabel(int idx);
        Sel* SelByNameTry(std::string sel);
        Sel* SelByName(std::string sel);
        void SetFloat(std::string sel, std::string param, float val);
        void SetString(std::string sel, std::string param, std::string val);
        std::string ParamVal(std::string sel, std::string param);

        void Apply(StylerObject *obj, bool setMsg);
    };
    
    typedef std::map<std::string, Sheet> Sheets; // Sheets is a map of named sheets -- used in the Set
    

    struct Set {
        std::string Desc; // description of this param set -- when should it be used?  how is it different from the other sets?
        Sheets SheetSet; // Sheet's grouped according to their target and / or function. For example, "Network" for all the network params (or "Learn" vs. "Act" for more fine-grained), and "Sim" for overall simulation control parameters, "Env" for environment parameters, etc.  It is completely up to your program to lookup these names and apply them as appropriate.

        Sheet* SheetByNameTry(std::string name);
        Sheet* SheetByName(std::string name);
        void ValidateSheets(std::vector<std::string> valids);
        void SetFloat(std::string sheet, std::string sel, std::string param, float val);
        void SetString(std::string sheet, std::string sel, std::string param, std::string val);
        std::string ParamValue(std::string sheet, std::string sel, std::string param);
    };
    
    typedef std::map<std::string, Set> Sets;

    // FlexVal is a specific flexible value for the Flex parameter map
    // that implements the StylerObj interface for CSS-style selection logic.
    // The field names are abbreviated because full names are used in StylerObj.
    struct FlexVal{
        std::string Nm, // name of this specific object, matches #Name selections
            Type, // type name of this object, matches plain TypeName selections
            Cls; // space-separated list of class name(s), match the .Class selections
        std::any Obj; // actual object with data that is set by the parameters
        // HistoryImpl History; // History of params applied

        // FlexVal(std::string nm, std::string tp, std::string cls, std::any obj):Nm(nm),Type(tp),Cls(cls),Obj(obj){};

        std::string TypeName(){return Type;};
        std::string Class(){return Cls;};
        std::string Name(){return Nm;};
        std::any &Object(){return Obj;}; // TODO Check for a more meaningful class of objects with string representation
        void CopyFrom(FlexVal cp);
        // std::string JSONString();
        // void ParamsHistoryReset();
        // void ParamsApplied();
    };
    void to_json(json& j, const FlexVal& flx);
    // void from_json(json& j, const FlexVal& flx);

    // Flex supports arbitrary named parameter values that can be set
    // by a Set of parameters, as a map of any objects.
    // First initialize the map with set of names and a type to create
    // blank values, then apply the Set to it.
    struct Flex{
        std::map<std::string, FlexVal> map;
        // Flex() //Initialize map?
        std::string TypeName(){return "Flex";};
        std::string Class(){return "";};
        std::string Name(){return "";};

        void Init(std::vector<FlexVal> vals);
        void ApplySheet(Sheet* sheet, bool setMsg);
        void CopyFrom(Flex cp);
        // void WriteJSON();
        std::string JSONString();
        // void SaveJSON(file);
    };

    std::any FindParam(std::any &val, std::string path);
    std::string SetParam(std::any &obj, std::string path, std::string val);
    float GetParam(std::any &obj, std::string path);
};