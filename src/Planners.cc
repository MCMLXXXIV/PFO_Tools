#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>

#include "Planners.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"

using namespace std;

//
// Fills out the plan.GateHead.GateTree
//
Plan* Planners::CreatePlanForItemsGoal(LineItem *goal,
				       Supply &bank,
				       TrackedResources &trackedResources,
				       Cost &cost)
{
    if (goal == NULL) {
	cout << "Planners::CreatePlanForItemsGoal() called with zero items" << endl;
	return NULL;
    }

    int callCount = 0;
    int maxDepth = 0;

    Plan *plan = new Plan();
    Gate *gate = GetPlanStep(goal, bank, trackedResources, cost, false, NULL, 1, maxDepth, callCount);

    plan->GateHead.GateTree.push_back(gate);

    plan->RecursionCallCount = callCount;
    plan->RecursionMaxDepth = maxDepth;
    return plan;
}

Gate* Planners::GetPlanStep(LineItem *req, 
			    Supply &bank,
			    TrackedResources &trackedResources,
			    Cost &cost,
			    bool productConsumed,
			    LineItem *parentLineItem,
			    int depth,
			    int &maxDepth,
			    int &callCount)
{
    char indent[(depth*4)+1];
    indent[depth*4] = '\0';
    memset(indent, ' ', depth*4);

    char buf[16];
    if (EntityTypeHelper::Instance()->QuantityIsWholeNumber(req->Entity->Type[0])) {
	snprintf(buf, 15, "%1.f", req->Quantity);
    } else {
	snprintf(buf, 15, "%.3f", req->Quantity);
    }
    cout << indent << "Planners::GetPlanStep(" << buf << " of " << req->Entity->Name << "); depth: " << depth << endl;

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
	    cout << indent << "BANK: " << req->Quantity << " of " << req->Entity->Name << " satisfied from bank" << endl;
	    return new Gate(isLeaf, bankFilled, req);
	}
	needed = stillNeeded;
    }
	
    int newGates = 0;

    // get the requirements for the requested rank
    // as of this writing achievements and feats have ranks
    // all the others (like Items, Time, etc) do not
    // though I am toying with the idea of ranking items too - to track +1, +2 things, etc
    list < LineItem* > *reqs = NULL;
    if (EntityTypeHelper::Instance()->IsRanked(req->Entity->Type[0])) {
	// I fear rounding error - but in the case of Ranked entities, the Qty should always
	// be a whole number.
	unsigned rank = unsigned(req->Quantity + 0.1);
	// TODO - until I finish the parsers, some entities will have incomplete data
	if (req->Entity->Requirements.size() < 1) {
	    cout << indent << "WARNING: this ranked Entity has no processed requirements; name: " << req->Entity->Name << endl;
	    reqs = new list< LineItem* >();
	} else if (req->Entity->Requirements.size() < rank) {
	    cout << indent << "WARNING: this ranked Entity has incomplete requirements; name: " << req->Entity->Name << endl;
	    reqs = new list< LineItem* >();
	} else {
	    reqs = &( req->Entity->Requirements[rank] );
	    cout << indent << "Have " << reqs->size() << " reqs for " << req->Entity->Name 
		 << " rank " << rank << " of " << (req->Entity->Requirements.size()-1) << endl;
	}	
    } else {
	if (req->Entity->Requirements.size() < 1) {
	    // for things like "Time" there may be no sub reqs at all
	    // in this case, make an empty req list so that we can continue normally
	    reqs = new list< LineItem* >();
	    cout << indent << "*** no requirements for " << req->Entity->Name << endl;
	} else {
	    reqs = &( req->Entity->Requirements[0] );
	}
    }

    bool willBeConsumed = productConsumed;
    double remainder = 0.0;
    int manufactureCycles = 1;
    list< LineItem* >::iterator reqEntry = reqs->begin();
    if (EntityTypeHelper::Instance()->IsType(req->Entity->Type[0], "Item")) {
	assert(req->Entity->CreationIncrement > 0);
	manufactureCycles = int(ceil(needed / req->Entity->CreationIncrement));
	remainder = (req->Entity->CreationIncrement * manufactureCycles) - needed;
	willBeConsumed = true;
    }
    for (; reqEntry != reqs->end(); reqEntry++) {
	LineItem *subReq = *reqEntry;
	if (!trackedResources.IsTracked(subReq->Entity->Type)) {
	    cout << indent << "skipping untracked requirement: " << subReq->Entity->Name << endl;
	    continue;
	}
	if (EntityTypeHelper::Instance()->IsType(subReq->Entity->Type[0], "LogicOr")) {
	    gate->SetIsOrGate(true);
	    cout << indent << req->Entity->Name << " skipping OR gate" << endl;
	    cost.Add("OR Gate");
	    continue;
	}

	LineItem *gateReq = new LineItem(*subReq);
	if (EntityTypeHelper::Instance()->IsType(subReq->Entity->Type[0], "Item") ||
	    EntityTypeHelper::Instance()->IsType(subReq->Entity->Type[0], "Time")) {
	    gateReq->Quantity = gateReq->Quantity * manufactureCycles;
	}
	
	//---------------------------------------------------------------------//
	//                            RECURSION HERE                           //
	//---------------------------------------------------------------------//
	(gate->GateTree).push_back(GetPlanStep(gateReq, bank, trackedResources, cost, willBeConsumed, req, depth+1, maxDepth, callCount));

	newGates++;	
    }

    if (EntityTypeHelper::Instance()->IsUniversal(req->Entity->Type[0])) {
	// for example, feats, achievements, ability scores
	string parentEntity = "";
	if (parentLineItem != NULL) {
	    parentEntity = " for " + parentLineItem->Entity->Name;
	}
	cout << indent << "BANK: deposit " << req->Quantity << " of " << req->Entity->Name << parentEntity << endl; 
	bank.Deposit(req);
    } else if (EntityTypeHelper::Instance()->IsType(req->Entity->Type[0], "Item") && productConsumed == false) {
	// for example, the item goal(s)
	bank.Deposit(req);
    }
    if (newGates < 1) {
	string costMessage;
	if (parentLineItem == NULL) {
	    costMessage = req->Entity->Name;
	} else {
	    char buf[255];
	    char *startHere = buf;
	    if (EntityTypeHelper::Instance()->QuantityIsWholeNumber(req->Entity->Type[0])) {
		int charsAdded = snprintf(startHere, (254 - (startHere-buf)), "%0.f for ", req->Quantity);
		startHere += charsAdded;
	    } else {
		int charsAdded = snprintf(startHere, (254 - (startHere-buf)), "%.3f for ", req->Quantity);
		startHere += charsAdded;
	    }
	    if (EntityTypeHelper::Instance()->QuantityIsWholeNumber(parentLineItem->Entity->Type[0])) {
		snprintf(startHere, (254 - (startHere-buf)), "%0.f of %s", parentLineItem->Quantity, parentLineItem->Entity->Name.c_str());
	    } else {
		snprintf(startHere, (254 - (startHere-buf)), "%.3f of %s", parentLineItem->Quantity, parentLineItem->Entity->Name.c_str());
	    }
	    costMessage = buf;
	}
	cost.Add(req, costMessage);
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

    cout << indent << "handled " << req->Quantity << " of " << req->Entity->Name << " with " << newGates << " new gates" << endl;
    return gate;

}



