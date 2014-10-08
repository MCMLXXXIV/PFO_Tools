#ifndef SUPPLY_H
#define SUPPLY_H

#include <string>
#include <list>

class EntityDefinition;

using namespace std;

class Supply {
 public:
    list<EntityDefinition> Items;
    string DummyFunction();
};


#endif
