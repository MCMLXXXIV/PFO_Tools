#ifndef TRACKEDRESOURCES_H
#define TRACKEDRESOURCES_H

#include <string>
#include <map>
#include <list>

using namespace std;

class TrackedResources {
 public:
    void SetTracked(list<short[]> tracked);
    bool IsTracked(short *type); // short array terminated by 0 val
 private:
    map<string, bool> TrackedByInternalTypeKey;
};

#endif
