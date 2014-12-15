#ifndef TRACKEDRESOURCES_H
#define TRACKEDRESOURCES_H

#include <string>
#include <map>
#include <list>

using namespace std;

class TrackedResources {
 public:
    void SetTracked(list<short*> tracked);
    bool IsTracked(short *type); // short array terminated by 0 val
    void DumpTrackedResources();
    static TrackedResources* Deserialize(const char*) { return NULL; };

 private:
    map<string, bool> TrackedByInternalTypeKey;
    map<string, bool> NotTrackedByInternalTypeKey;
};

#endif
