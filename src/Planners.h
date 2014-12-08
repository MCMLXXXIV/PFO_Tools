#ifndef PLANNERS_H
#define PLANNERS_H

#include <list>

class EntityDefinition;
class Supply;
class TrackedResources;
class Cost;
class Plan;
class OfficialData;
class LineItem;
class Gate;

using namespace std;

class Planners {
 public:
    static Plan* CreatePlanForItemsGoal(LineItem *items,
				       Supply &bank,
				       TrackedResources &trackedResources,
				       Cost &cost
				       );
    static string CreatePlanForItemGoalForWeb(const char *input);

 private:
    static Gate* GetPlanStep(LineItem *req,
			     Supply &bank,
			     TrackedResources &trackedResources,
			     Cost &cost,
			     bool productConsumed,
			     LineItem *parentLineItem,
			     int depth,
			     int &maxDepth,
			     int &callCount);
};


#endif
