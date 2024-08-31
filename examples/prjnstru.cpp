#include "prjnstru.hpp"

// Connect sets the connectivity between two layers and the pattern to use in interconnecting them
void leabra::PrjnStru::Connect(Layer& slay, Layer& rlay, emer::Pattern& pat, emer::PrjnType& typ) {
    Send = &slay;
    Recv = &rlay;
    Pat = &pat;
    Typ = &typ;
}

// Validate tests for non-nil settings for the projection -- returns error
// message or nil if no problems (and logs them if logmsg = true)
void leabra::PrjnStru::Validate(bool logmsg) {
    std::string emsg = "";
    if (Pat == nullptr){
        emsg += "Pat is nil; ";
    };
    if (Recv == nullptr){
        emsg += "Recv is nil; ";
    };
    if (Send == nullptr){
        emsg += "Send is nil; ";
    };
    if (emsg != ""){
        if (logmsg){
            std::cerr << emsg << std::endl;
        }
        else{
            throw std::runtime_error(emsg);
        }
    };
}

// BuildStru constructs the full connectivity among the layers as specified in this projection.
// Calls Validate and returns false if invalid.
// Pat.Connect is called to get the pattern of the connection.
// Then the connection indexes are configured according to that pattern.
void leabra::PrjnStru::BuildStru() {
    if (Off){
        return;
    }
    Validate(true);
    tensor::Shape& ssh = Send->Shp;
    tensor::Shape& rsh = Recv->Shp;
    tensor::Int32 sendn, recvn;
    Pat->Connect(); // TODO FINISH
    int slen = ssh.Len();
    int rlen = rsh.Len();
    int tcons = SetNIdxSt(SConN, SConNAvgMax, SConIdxSt, sendn);
    int tconr = SetNIdxSt(SConN, SConNAvgMax, SConIdxSt, recvn);
}

// SetNIdxSt sets the *ConN and *ConIdxSt values given n tensor from Pat.
// Returns total number of connections for this direction.
int leabra::PrjnStru::SetNIdxSt(std::vector<int> &n, minmax::AvgMax32 avgmax, std::vector<int> idxst, etensor::Int32 tn) {
    int ln = tn.Len();
    int idx = 0;
    for (int i = 0; i < ln; i++){
        int nv = tn.Values[i];
        n.push_back(nv);
        idx += nv;
        avgmax.UpdateValue((float)nv, i);
    }
    avgmax.CalcAvg();
    return idx;
}

std::string leabra::PrjnStru::String() {
    std::string str= "";
    if (Recv == nullptr){
        str += "recv=nil;";
    } else {
        str += Recv->Nm + " <- ";
    }
    if (Send == nullptr){
        str += "send=nil;";
    } else {
        str += Send->Nm;
    }
    if (Pat == nullptr){
        str += "Pat=nil;";
    } else {
        str += "Pat=" + Pat->Name();
    }
    return str;
}

// ApplyParams applies given parameter style Sheet to this projection.
// Calls UpdateParams if anything set to ensure derived parameters are all updated.
// If setMsg is true, then a message is printed to confirm each parameter that is set.
// it always prints a message if a parameter fails to be set.
// returns true if any params were set, and error if there were any errors.
bool leabra::PrjnStru::ApplyParams(params::Sheet &pars, bool setMsg) {
    std::any cont = std::any(this);
    pars.Apply(cont, setMsg);
    UpdateParams();
    return true; // TODO maybe change this... might be outdated anyways
}

// std::string leabra::PrjnStru::NonDefaultParams() {
//     return std::string();
// }
