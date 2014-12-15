#ifndef PLAN_H
#define PLAN_H

#include <string>

#include "Gate.h"

using namespace std;

class Plan {
 public:
    string DummyFunction();
    Gate GateHead;
    int RecursionCallCount;
    int RecursionMaxDepth;

    static string SerializeJson(Plan *plan);
};

#endif
