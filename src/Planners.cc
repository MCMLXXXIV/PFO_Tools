#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>
#include <ctime>


#include "Planners.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "Log.h"
#include "OfficialData.h"

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

    Gate *gate = GetPlanStep(goal, bank, trackedResources, cost, false, NULL, 1, maxDepth, callCount);

    Plan *plan = new Plan();
    plan->GateHead.GateTree.push_back(gate);
    plan->RecursionCallCount = callCount;
    plan->RecursionMaxDepth = maxDepth;
    return plan;
}

string Planners::CreatePlanForItemGoalForWeb(const char *itemName, unsigned rank, Supply *store, TrackedResources *tracked) {	
    time_t now = time(NULL);
    struct tm * timeinfo;
    timeinfo = localtime(&now);

    char timebuf[1024];
    strftime(timebuf, 1023, "%Y%m%d_%H%M%S", timeinfo);

    // EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    EntityDefinition *entity = OfficialData::Instance()->GetEntity(itemName);
    if (entity == NULL) {
	char buf[1024];
	snprintf(buf, 1023, "[ { \"plainText\": \"%s\", \"ItemDoesNotExist\": \"%s\" } ]", timebuf, itemName);
	return buf;
    }

    LineItem *item = new LineItem(entity, rank, 1.0);

    Cost cost;
    Plan *plan = CreatePlanForItemsGoal(item, *store, *tracked, cost);

    string retVal = "[ { \"plainText\": \"" + string(timebuf) + "\", ";
    retVal += "\"Item\": \"" + string(itemName) + "\", ";
    retVal += "\"Bank\": " + Supply::SerializeJson(store) + ", ";
    retVal += "\"Cost\": " + Cost::SerializeJson(&cost) + ", ";
    retVal += "\"Plan\": " + Plan::SerializeJson(plan) + ", ";
    retVal += "\"ItemDeep\": " + EntityDefinition::SerializeJson(entity);
    retVal += " } ]";

    return retVal.c_str();
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

    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();
    Logger *log = Logger::Instance();

    log->Log(Logger::Level::Verbose, "Planners",
	     "%sPlanners::GetPlanStep(%s); depth: %d\n",
	     indent, req->ToString().c_str(), depth);

    Gate *gate = new Gate();
    
    // strictly for accounting/profiling
    if (depth > maxDepth) { maxDepth = depth; }
    ++callCount;

    // because we may partially satisfy this requirement from the bank, we need to make a copy
    // of the req LineItem to keep track of how much we still need after the bank withdrawal
    LineItem needed(*req);
    bool satisfiedByBank = bank.Withdrawal(&needed);
    if (satisfiedByBank == true) {
	bool isLeaf = true;
	bool bankFilled = true;
	log->Log(Logger::Level::Verbose, "Planners",
		 "%sBank: %s satisfied from bank\n",
		 indent, req->ToString().c_str());
	return new Gate(isLeaf, bankFilled, req);
    }

    int newGates = 0;

    // set to true below when this req is for rank > 0 but this item has requirements only for rank 0
    bool rankOverride = false;

    // get the requirements for the requested rank
    list < LineItem* > *reqs = NULL;
    if (eTypeHelper->IsRanked(req->Entity->Type[0]) || eTypeHelper->IsType(req->Entity->Type[0], "Item")) {
	unsigned rank = req->Rank;
	// TODO - until I finish the parsers, some entities will have incomplete data
	if (req->Entity->Requirements.size() < 1) {
	    log->Log(Logger::Level::Note, "Planners",
		     "%sthis ranked Entity has no listed Requirements; name: %s\n",
		     indent, req->Entity->Name.c_str());
	    reqs = new list< LineItem* >();
	} else if (req->Entity->Requirements.size() == 1 && rank > 0) {
	    // this is for handling items with pluses.  You make them the same as the normal, +0, items
	    // but with "plus" ingredients.  EG, +2 longswords - you make them the same as +0 longswords
	    // but with +2 components.
	    log->Log(Logger::Level::Verbose, "Planners",
		     "%sthis Entity only has requirements for rank 0 but the requirement is for rank %d; - going to bump the component ranks; name: %s\n",
		     indent, rank, req->Entity->Name.c_str());
	    reqs = &( req->Entity->Requirements[0] );
	    rankOverride = true;
	} else if (req->Entity->Requirements.size() <= rank) {
	    log->Log(Logger::Level::Warn, "Planners",
		     "%sthis ranked Entity only has requirements for %d ranks (so up to rank %d) but the requirement is for rank %d; name: %s\n",
		     indent, req->Entity->Requirements.size(), req->Entity->Requirements.size() - 1, rank, req->Entity->Name.c_str());
	    reqs = new list< LineItem* >();
	} else {
	    reqs = &( req->Entity->Requirements[rank] );
	    log->Log(Logger::Level::Verbose, "Planners",
		     "%sProcessing %d requirements for %s rank %d of %d\n",
		     indent, reqs->size(), req->Entity->Name.c_str(), rank, (req->Entity->Requirements.size()-1));
	}	
    } else {
	if (req->Entity->Requirements.size() < 1) {
	    // for things like "Time" there may be no sub reqs at all
	    // in this case, make an empty req list so that we can continue normally
	    reqs = new list< LineItem* >();
	    log->Log(Logger::Level::Verbose, "Planners",
		     "%s*** no requirements for %s\n",
		     indent, req->Entity->Name.c_str());
	} else {
	    reqs = &( req->Entity->Requirements[0] );
	}
    }

    bool willBeConsumed = productConsumed;
    double remainder = 0.0;
    int manufactureCycles = 1;
    auto reqEntry = reqs->begin();
    if (eTypeHelper->IsType(req->Entity->Type[0], "Item")) {
	// remember: uses "needed" here because we may have satisfied some of the req from the bank
	// and that will only be reflected in the "needed" element
	double qtyRequired = needed.Quantity;
	assert(req->Entity->CreationIncrement > 0);
	manufactureCycles = int(ceil(qtyRequired / req->Entity->CreationIncrement));
	remainder = (req->Entity->CreationIncrement * manufactureCycles) - qtyRequired;
	willBeConsumed = true;
	log->Log(Logger::Level::Verbose, "Planners",
		 "%swe need %s, item created in increments of %d, must pay for %d cycles and make %d\n",
		 indent, needed.ToString().c_str(), req->Entity->CreationIncrement, manufactureCycles, (req->Entity->CreationIncrement * manufactureCycles));
    }
    for (; reqEntry != reqs->end(); reqEntry++) {
	LineItem *subReq = *reqEntry;
	if (!trackedResources.IsTracked(subReq->Entity->Type)) {
	    log->Log(Logger::Level::Note, "Planners",
		     "%sskipping untracked requirement: %s\n",
		     indent, subReq->Entity->Name.c_str());
	    continue;
	}
	if (eTypeHelper->IsType(subReq->Entity->Type[0], "LogicOr")) {
	    gate->SetIsOrGate(true);
	    log->Log(Logger::Level::Note, "Planners",
		     "%s%s skipping OR gate\n",
		     indent, req->Entity->Name.c_str());
	    cost.Add(subReq->Describe(req));
	    continue;
	}

	LineItem *gateReq = new LineItem(*subReq);
	if (eTypeHelper->IsType(subReq->Entity->Type[0], "Item")) {
	    gateReq->Quantity = gateReq->Quantity * manufactureCycles;
	    if (rankOverride) {
		gateReq->Rank = req->Rank;
	    }
	} else if (eTypeHelper->IsType(subReq->Entity->Type[0], "Time")) {
	    gateReq->Quantity = gateReq->Quantity * manufactureCycles;
	}

	
	//---------------------------------------------------------------------//
	//                            RECURSION HERE                           //
	//---------------------------------------------------------------------//
	(gate->GateTree).push_back(GetPlanStep(gateReq, bank, trackedResources, cost, willBeConsumed, req, depth+1, maxDepth, callCount));

	newGates++;	
    }

    if (eTypeHelper->IsUniversal(req->Entity->Type[0])) {
	// for example, feats, achievements, ability scores
	string parentEntity = "";
	if (parentLineItem != NULL) {
	    parentEntity = " for " + parentLineItem->Entity->Name;
	}
	log->Log(Logger::Level::Note, "Planners",
		 "%sBANK: deposit %s%s\n",
		 indent, req->ToString().c_str(), parentEntity.c_str());
	bank.Deposit(req);
    } else if (eTypeHelper->IsType(req->Entity->Type[0], "Item") && productConsumed == false) {
	// for example, the item goal(s)
	bank.Deposit(req);
    }
    if (newGates < 1) {
	string costMessage = req->ToString();
	if (parentLineItem != NULL) {
	    costMessage += " for " + parentLineItem->ToString();
	}
	cost.Add(req, costMessage);
    }

    // only add the remainder if there is one - that is, if we created the item.  And we will
    // only have created the item if there were new gates added.
    if (eTypeHelper->IsType(req->Entity->Type[0], "Item")) {
	// there will only be remainders for items
	if (remainder != 0.0) {
	    assert(remainder >= 1.0); // I fear rounding error
	    string parentEntity = "";
	    if (parentLineItem != NULL) {
		parentEntity = " for " + parentLineItem->Entity->Name;
	    }
	    LineItem *newBankItem = new LineItem(req->Entity, req->Rank, remainder);
	    log->Log(Logger::Level::Note, "Planners",
		     "%sBANK: deposit remainder %f of %s%s (we used %f)\n",
		     indent, remainder, req->Entity->Name.c_str(), parentEntity.c_str(), needed.Quantity);
	    bank.Deposit(newBankItem);
	}
    }

    log->Log(Logger::Level::Note, "Planners",
	     "%shandled %s with %d new gates (%lu)\n",
	     indent, req->ToString().c_str(), newGates, (unsigned long)gate);
    return gate;

}



