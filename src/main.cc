#include <cassert>
#include <iostream>

#include "EntityTypeHelper.h"
#include "OfficialData.h"
#include "Planners.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "Utils.h"

using namespace std;

int main() {
    
    OfficialData rulesGraph;
    rulesGraph.ProcessSpreadsheetDir("official_data");

    EntityDefinition *headEntity = new EntityDefinition();
    headEntity->Name = "LogicAnd";
    list<string> typeStringName;
    typeStringName.push_back("LogicAnd");

    headEntity->Type = EntityTypeHelper::Instance()->GetType(typeStringName);

    // entity = rulesGraph.GetEntity("Pot Steel Plate");
    // entity = rulesGraph.GetEntity("Journeyman's Speed Potion");
    
    headEntity->Requirements.push_back(list<LineItem*>());

    EntityDefinition *entity = rulesGraph.GetEntity("Hunter's Longbow");
    assert(entity != NULL);
    LineItem *stuff = new LineItem(entity, 2.0);
    headEntity->Requirements[0].push_back(stuff);

    entity = rulesGraph.GetEntity("Yew and Iron Splint");
    assert(entity != NULL);
    stuff = new LineItem(entity, 1.0);
    headEntity->Requirements[0].push_back(stuff);

    LineItem *headGoal = new LineItem(headEntity, 1.0);

    Supply bank;

    TrackedResources trackedResources;

    list<short*> trackedTypes;

    typeStringName.clear();
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
    plan = Planners::CreatePlanForItemsGoal(headGoal, bank, trackedResources, cost);

    cout << endl;
    cout << "Cost:" << endl;
    cost.Dump();
    cout << endl;

    cout << "Final Supply:" << endl;
    bank.Dump();
    cout << endl;

    return 0;
}


void TestUtilsSplit() {

    list<string> foo = Utils::SplitCommaSeparatedValuesWithQuotedFields("foo,bar ,\"choo,crab\", ,blue, green, \"in, out \", red \"and, this\", "" , \"rog ");
    list<string>::iterator fItr;
    int idx = 0;
    for (fItr = foo.begin(); fItr != foo.end(); ++fItr) {
	cout << idx << " [" << *fItr << "]" << endl;
	++idx;
    }
    return;
}
