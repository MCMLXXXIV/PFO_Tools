#include "WebClientRequestArgs.h"

using namespace std;

void WebClientRequestArgs::AddKeyVal(string key, string val, bool append) {
    if (ArgMap.find(key) == ArgMap.end()) {
	ArgMap[key] = val;
    } else {
	if (append) {
	    string newVal = ArgMap[key];
	    newVal += val;
	    ArgMap[key] = val;
	} else {
	    printf("WARNING: we got another value for the key %s - I don't handle this right now so I'm ignoring it: FirstVal: [%s]; IgnoredVal: [%s]\n", key.c_str(), ArgMap[key].c_str(), val.c_str());
	}
    }
}
