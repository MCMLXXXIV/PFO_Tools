#ifndef WEBCLIENTREQUESTARGS_H
#define WEBCLIENTREQUESTARGS_H

// I'm adding this because a simple key/val map became insuffient when I started to handle
// priority lists for resolving OR requirement nodes

#include <map>
#include <string>

class WebClientRequestArgs {
 public:
    std::map<std::string, std::string> ArgMap;

    void AddKeyVal(std::string key, std::string val, bool append = false);
};

#endif
