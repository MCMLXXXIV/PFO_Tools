#ifndef COST_H
#define COST_H

#include <string>
#include <list>
#include <map>

class LineItem;

using namespace std;

class Cost {
 public:
    void Add(LineItem *item, string msg);
    void Add(string msg);
    void Dump();

 private:
    void Add(LineItem *item, string msg, int level);
    void Dump(int);

    list<string> NonAggregateCosts;
    map<short, Cost*> SubNodes;
    short *Type;
    list<string> Notes;
    double Sum;
};

#endif
