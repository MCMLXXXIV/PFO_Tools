#include "Plan.h"

string Plan::DummyFunction() { return "foo"; }

string Plan::SerializeJson(Plan *plan) {
    string retVal = "{ \"RecursionMaxDepth\": ";
    retVal += to_string(plan->RecursionMaxDepth);
    retVal += ", \"RecursionCallCount\": ";
    retVal += to_string(plan->RecursionCallCount);
    // retVal += ", \"PlanAddr\": " + to_string((unsigned long)plan);
    retVal += ", \"GateHead\": ";
    retVal += Gate::SerializeJson(&(plan->GateHead));
    retVal += " }";

    return retVal;
}
