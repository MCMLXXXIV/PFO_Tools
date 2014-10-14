#include <iostream>
#include <cmath>
#include <cassert>

#include "Planners.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "OfficialData.h"

using namespace std;

//
// Fills out the plan.GateHead.GateTree
//
Plan* Planners::CreatePlanForItemsGoal(LineItem *goal,
				      Supply &bank,
				      TrackedResources &trackedResources,
				      Cost &cost,
				      OfficialData &rulesGraph)
{
    if (goal == NULL) {
	cout << "Planners::CreatePlanForItemsGoal() called with zero items" << endl;
	return NULL;
    }

    int callCount = 0;
    int maxDepth = 0;

    Plan *plan = new Plan();
    Gate *gate = GetPlanStep(goal, bank, trackedResources, cost, rulesGraph, 1, maxDepth, callCount);
    plan->GateHead.GateTree.push_back(gate);

    plan->RecursionCallCount = callCount;
    plan->RecursionMaxDepth = maxDepth;
    return plan;
}

Gate* Planners::GetPlanStep(LineItem *req, 
			    Supply &bank,
			    TrackedResources &trackedResources,
			    Cost &cost,
			    OfficialData &rulesGraph,
			    int depth,
			    int &maxDepth,
			    int &callCount)
{
    
    cout << "Planners::GetPlanStep(" << req->Entity->Name << ")" << endl;

    Gate *gate = new Gate();
    
    // strictly for accounting/profiling
    if (depth > maxDepth) { maxDepth = depth; }
    ++callCount;

    double needed = req->Quantity;
    if (bank.Has(req->Entity)) {
	double stillNeeded = bank.Withdrawal(req);
	if (stillNeeded <= 0.0) {
	    bool isLeaf = true;
	    bool bankFilled = true;
	    cout << req->Quantity << " of " << req->Entity->Name << " satisfied from bank" << endl;
	    return new Gate(isLeaf, bankFilled, req);
	}
	needed = stillNeeded;
    }
	
    Gate *newGate = new Gate(req);
    int newGates = 0;

    // get the requirements for the requested rank
    // as of this writing, skills, achievements, and feats have ranks
    // all the other (like Items, Time, etc) do not
    // though I am toying with the idea of ranking items too - to track +1, +2 things, etc
    list < LineItem* > *reqs = NULL;
    if (EntityTypeHelper::Instance()->IsRanked(req->Entity->Type[0])) {
	// I fear rounding error - but in the case of Ranked entities, the Qty should always
	// be a whole number.
	unsigned rank = unsigned(req->Quantity + 0.1);
	// TODO - until I finish the parsers, some entities will have incomplete data
	if (req->Entity->Requirements.size() < 1) {
	    cout << "WARNING: this ranked Entity has no processed requirements; name: " << req->Entity->Name << endl;
	    reqs = new list< LineItem* >();
	} else if (req->Entity->Requirements.size() < rank) {
	    cout << "WARNING: this ranked Entity has incomplete requirements; name: " << req->Entity->Name << endl;
	    reqs = new list< LineItem* >();
	} else {
	    reqs = &( req->Entity->Requirements[rank] );
	}	
    } else {
	if (req->Entity->Requirements.size() < 1) {
	    // for things like "Time" (or until I finish this program, Skills), there may be no sub reqs at all
	    // in this case, make an empty req list so that we can continue normally
	    reqs = new list< LineItem* >();
	} else {
	    reqs = &( req->Entity->Requirements[0] );
	}
    }

    double remainder = 0.0;
    int manufactureCycles = 1;
    list< LineItem* >::iterator reqEntry = reqs->begin();
    if (EntityTypeHelper::Instance()->IsType(req->Entity->Type[0], "Item")) {
	assert(req->Entity->CreationIncrement > 0);
	manufactureCycles = int(ceil(needed / req->Entity->CreationIncrement));
	remainder = (req->Entity->CreationIncrement * manufactureCycles) - needed;
    }
    for (; reqEntry != reqs->end(); reqEntry++) {
	LineItem *subReq = *reqEntry;
	if (!trackedResources.IsTracked(subReq->Entity->Type)) { continue; }
	if (EntityTypeHelper::Instance()->IsType(subReq->Entity->Type[0], "LogicOr")) {
	    gate->SetIsOrGate(true);
	    cout << req->Entity->Name << " skipping or gate" << endl;
	    return gate;
	}

	LineItem *gateReq = new LineItem(*subReq);
	if (EntityTypeHelper::Instance()->IsType(subReq->Entity->Type[0], "Item")) {
	    gateReq->Quantity = gateReq->Quantity * manufactureCycles;
	}
	
	(gate->GateTree).push_back(GetPlanStep(gateReq, bank, trackedResources, cost, rulesGraph, depth+1, maxDepth, callCount));
	newGates++;	
    }

    if (EntityTypeHelper::Instance()->IsUniversal(req->Entity->Type[0])) {
	// for example, skills, achievements, ability scores
	bank.Deposit(req);
    }
    if (newGates < 1) {
	cost.Add(req);
    }

    // only add the remainder if thre is one - that is, if we created the item.  And we will
    // only have created the item if there were new gates added.
    if (EntityTypeHelper::Instance()->IsType(req->Entity->Type[0], "Item")) {
	// there will only be remainders for items
	if (remainder != 0.0) {
	    assert(remainder >= 1.0); // I fear rounding error
	    LineItem *newBankItem = new LineItem(req->Entity, remainder);
	    bank.Deposit(newBankItem);
	}
    }

    cout << "handled " << req->Quantity << " of " << req->Entity->Name << " with " << newGates << " new gates" << endl;
    return gate;

}



