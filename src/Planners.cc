#include <iostream>

#include "Planners.h"
#include "EntityDefinition.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "OfficialData.h"

using namespace std;

bool Planners::CreatePlanForItemsGoal(list<EntityDefinition> items,
				      Supply bank,
				      TrackedResources trackedResources,
				      Cost &cost,
				      Supply &remainder,
				      Plan &plan,
				      OfficialData &rulesGraph)
{
    if (items.size() < 1) {
	cout << "Planners::CreatePlanForItemsGoal() called with zero items" << endl;
	return true;
    }

    list<EntityDefinition>::iterator itr;
    for(itr = items.begin(); itr != items.end(); itr++) {
	CreatePlanForItemGoal(*itr, bank, trackedResources, cost, plan, rulesGraph);
    }

    return true;
}

bool Planners::CreatePlanForItemGoal(EntityDefinition &item, Supply &bank, TrackedResources &trackedResources,
				     Cost &cost, Plan &plan, OfficialData &rulesGraph)
{

    cout << "Planners::CreatePlanForItemGoal(" << item.Name << ")" << endl;
    return true;

}
