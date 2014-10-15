#include <cassert>
#include <iostream>

#include "EntityTypeHelper.h"
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
    // entity = rulesGraph.GetEntity("Pot Steel Plate");
    // entity = rulesGraph.GetEntity("Journeyman's Speed Potion");
    assert(entity != NULL);
    
    LineItem *stuff = new LineItem(entity, 2.0);
    Supply bank;

    TrackedResources trackedResources;
    list<string> typeStringName;

    list<short*> trackedTypes;

    typeStringName.push_back("Item");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("Skill");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("Time");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("ExperiencePoint");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    trackedResources.SetTracked(trackedTypes);

    Cost cost;
    Supply remainder;
    Plan *plan;
    plan = Planners::CreatePlanForItemsGoal(stuff, bank, trackedResources, cost);

    cout << endl;
    cout << "Cost:" << endl;
    cost.Dump();
    cout << endl;

    cout << "Final Supply:" << endl;
    bank.Dump();
    cout << endl;

    return 0;
}
