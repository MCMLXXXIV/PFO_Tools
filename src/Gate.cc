#include <iostream>

#include "Gate.h"

Gate::Gate(LineItem *item) {
    Item = new LineItem(*item);
}

Gate::Gate(bool isLeaf, bool isBankFilled, LineItem *item) {
    IsLeaf = isLeaf;
    IsBankFilled = isBankFilled;
    Item = new LineItem(*item);
}

void Gate::SetIsOrGate(bool val) {
    IsOrGate = val;
}

string Gate::SerializeJson(Gate *gate) {
    string retVal = "{ \"LineItem\": ";
    retVal += LineItem::SerializeJson(gate->Item);
    // retVal += ", \"GateAddr\": " + to_string((unsigned long)gate);
    retVal += ", \"Children\": [ ";

    bool addSep = false;
    list<Gate*>::iterator childGateEntry = gate->GateTree.begin();
    for (; childGateEntry != gate->GateTree.end(); ++childGateEntry) {
	if (addSep) { retVal += ", "; } else { addSep = true; }
	retVal += Gate::SerializeJson(*childGateEntry);
    }
    retVal += " ]";  // end of children
    
    retVal += " }";  // end of this object

    //retVal += to_string(plan->RecursionMaxDepth);

    return retVal;
}
