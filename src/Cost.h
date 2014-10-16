#ifndef COST_H
#define COST_H

#include <string>
#include <list>
#include <map>

class LineItem;
class CostNode;

using namespace std;

class Cost {
 public:
    void Add(LineItem *item);
    // void Add(LineItem *item);
    void Dump();

 private:
    // typeid to costNode
    map<short, CostNode*> CostList;
    list< LineItem* > TempCostList;
};

#endif
