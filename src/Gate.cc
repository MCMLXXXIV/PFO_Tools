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
