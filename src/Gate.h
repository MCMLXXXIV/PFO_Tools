#ifndef GATE_H
#define GATE_H

#include <list>

#include "LineItem.h"

using namespace std;

class Gate : public LineItem
{
 public:
    Gate() {};
    Gate(LineItem *item);
    Gate(bool isLeaf, bool isBankFilled, LineItem *item);
    list<Gate*> GateTree;
    void SetIsOrGate(bool val);

 private:
    bool IsOrGate;
    bool IsLeaf;
    bool IsBankFilled;
    LineItem *Item;
};

#endif
