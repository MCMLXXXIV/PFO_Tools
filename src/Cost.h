#ifndef COST_H
#define COST_H

#include <string>
#include <list>

class LineItem;

using namespace std;

class Cost {
 public:
    void Add(LineItem *item);
    void Dump();

 private:
    list< LineItem* > TempCostList;
};

#endif
