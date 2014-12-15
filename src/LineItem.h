#ifndef LINEITEM_H
#define LINEITEM_H

#include <string>

using namespace std;

class EntityDefinition;

class LineItem {
 public:
    LineItem(){};
    LineItem(EntityDefinition* entity, double quantity);
    void Dump();
    string Describe(LineItem *parent);

    static string SerializeJson(LineItem *lineItem);

    EntityDefinition* Entity;
    double Quantity;
};

#endif
