#include <cassert>

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

    EntityDefinition *entity = rulesGraph.GetEntity("Hunter's Longbow");
    assert(entity != NULL);
    
    LineItem *stuff = new LineItem(entity, 1.0);
    Supply bank;
    TrackedResources trackedResources;
    Cost cost;
    Supply remainder;
    Plan *plan;
    plan = Planners::CreatePlanForItemsGoal(stuff, bank, trackedResources, cost, rulesGraph);

    return 0;
}
