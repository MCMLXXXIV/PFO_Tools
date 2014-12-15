#ifndef GATE_H
#define GATE_H

#include <list>

#include "LineItem.h"

using namespace std;

class Gate
{
 public:
    Gate() { Item = NULL; };
    Gate(LineItem *item);
    Gate(bool isLeaf, bool isBankFilled, LineItem *item);
    list<Gate*> GateTree;
    void SetIsOrGate(bool val);
    static string SerializeJson(Gate *gate);

 private:
    bool IsOrGate;
    bool IsLeaf;
    bool IsBankFilled;
    LineItem *Item;
};

#endif
