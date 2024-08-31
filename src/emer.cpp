#include "emer.hpp"

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
