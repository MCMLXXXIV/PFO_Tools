#ifndef PLANNERS_H
#define PLANNERS_H

#include <list>

class EntityDefinition;
class Supply;
class TrackedResources;
class Cost;
class Plan;
class OfficialData;

using namespace std;

class Planners {
 public:
    static bool CreatePlanForItemsGoal(list<EntityDefinition> items,
				       Supply bank,
				       TrackedResources trackedResources,
				       Cost &cost,
				       Supply &remainder,
				       Plan &plan,
				       OfficialData &rulesGraph);
 private:
    static bool CreatePlanForItemGoal(EntityDefinition &item,
				      Supply &bank,
				      TrackedResources &trackedResources,
				      Cost &cost,
				      Plan &plan,
				      OfficialData &rulesGraph);
};


#endif
