#ifndef HIERARCHICALID_H
#define HIERARCHICALID_H

#include <string>
#include <map>

using namespace std;

class HierarchicalId {
 public:
    HierarchicalId();
    HierarchicalId(string name, int index);
    string Name;
    int Index;
    map<string, HierarchicalId*> NextLevel;
};

#endif
