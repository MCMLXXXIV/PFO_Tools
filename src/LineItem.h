#ifndef LINEITEM_H
#define LINEITEM_H

#include <string>

using namespace std;

class EntityDefinition;

class LineItem {
 public:
    LineItem();
    LineItem(EntityDefinition* entity, int rank, double quantity);
    void Dump();
    string ToString();
    string Describe(LineItem *parent);

    static string SerializeJson(LineItem *lineItem);

    EntityDefinition* Entity;
    unsigned Rank;
    double Quantity;
};

#endif
