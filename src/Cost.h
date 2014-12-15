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

    static string SerializeJson(Cost *cost);

 private:
    void Add(LineItem *item, string msg, int level);
    void Dump(int);

    short *Type;
    double Sum;
    map<short, Cost*> SubNodes;
    list<string> Notes;

    list<string> NonAggregateCosts;
};

#endif
