#include "HierarchicalId.h"

HierarchicalId::HierarchicalId(string name, int index) {
    Name = name;
    Index = index;
}

HierarchicalId::HierarchicalId() {
    Name = "DUMMY";
    Index = 0;
}
