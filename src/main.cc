#include "OfficialData.h"
#include "Planners.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"

using namespace std;

int main() {

    OfficialData rulesGraph;
    rulesGraph.ProcessSpreadsheetDir("official_data");

    list<EntityDefinition> stuff;
    Supply bank;
    TrackedResources trackedResources;
    Cost cost;
    Supply remainder;
    Plan plan;
    Planners::CreatePlanForItemsGoal(stuff, bank, trackedResources, cost, remainder, plan, rulesGraph);

    return 0;
}
