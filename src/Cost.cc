#include "Cost.h"
#include "LineItem.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

#include <iostream>
#include <vector>

using namespace std;

void Cost::Add(string nonAggregatableCost) {
    NonAggregateCosts.push_back(nonAggregatableCost);
}

void Cost::Add(LineItem *item, string msg) {
    if (item == NULL) { return; }
    Add(item, msg, 0);
}

void Cost::Add(LineItem *item, string msg, int level) {
    if (item == NULL) { return; }

    // by virtue of the fact that we only work on subnodes, the root node is treated the
    // the way we need it to be treated - it is ignored.  The point of the nodes is to
    // tally and sum a hierarchy of items but the items are only logically combinable
    // at the second level and below.  That is, adding 300 seconds to 10 yew logs doesn't
    // make 310 of anything.

    EntityDefinition *entity = item->Entity;
    short idNextLevel = *((entity->Type) + level);
    if (idNextLevel == 0) { return; }

    Cost *childNode = SubNodes[idNextLevel];
    if (childNode == NULL) {
	childNode = new Cost();
	SubNodes[idNextLevel] = childNode;
	// if level 0, type is [id0, 0]
	// if level 1, type is [id0, id1, 0]
	// if level 2, type is [id0, id1, id2, 0]
	childNode->Type = new short[level+2];
	for (int idx = 0; idx < level + 1; ++idx) {
	    (childNode->Type)[idx] = (entity->Type)[idx];
	}
	(childNode->Type)[level + 1] = 0;
    }
    
    childNode->Notes.push_back(msg);

    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    if (eTypeHelper->IsRanked((entity->Type)[0])) {
	// achievements and feats
	if (childNode->Rank < item->Rank) {
	    childNode->Rank = item->Rank;
	}
    } else if (eTypeHelper->IsUniversal((entity->Type)[0])) {
	// AbilityScore and AchievementPoint
	// these are handled differently than feats and achievements because they
	// only have a quantity, not a rank.
	if (childNode->Quantity < item->Quantity) {
	    childNode->Quantity = item->Quantity;
	}
    } else {
	// Item, XP, Recipe, Time
	childNode->Quantity += item->Quantity;
	childNode->Rank = item->Rank;

	// right now I only handle one rank of an item.  IE, with the data
	// structure I have now for Cost, I can't track Steel Wire +1 and 
	// Steel Wire +2 at the same time.  This is fine for simple plans
	// and simple initial supplies.  But later I want to add some smarts
	// to the planner to take advatage of a varied supply.  EG, say I
	// have one or two +4 Steel Plates and I want to plan a +2 Steel
	// Longsword - I'd like the planner to use the +4 Steel Plates and
	// figure out the minimum rank required for the remaining components.
	// But I'm putting off that work for now.

    }

    childNode->Add(item, msg, level + 1);
}

void Cost::Dump() {
    Dump(1);
    if (NonAggregateCosts.size() > 0) {
	cout << endl << "Unaggregated costs:" << endl;
	list<string>::iterator itr;
	for (itr = NonAggregateCosts.begin(); itr != NonAggregateCosts.end(); ++itr) {
	    cout << "    " << *itr << endl;
	}
    }
}

void Cost::Dump(int level) {
    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    string indent;
    for (int x = 0; x < level; ++x) {
	indent += "    ";
    }

    auto itr = SubNodes.begin();
    for (; itr != SubNodes.end(); ++itr) {
	short *type = (*itr).second->Type;
	list<string> typeList = eTypeHelper->GetType(type);
	cout << indent << typeList.back() << ":";

	// only show the summed quantity for Time and ExperiencePoints
	if (level == 1 && (eTypeHelper->IsType(type[0],"Time") || eTypeHelper->IsType(type[0],"ExperiencePoint"))) {
	    cout << " " << (*itr).second->Quantity;
	    if (eTypeHelper->IsType(type[0], "ExperiencePoint")) {
		char buf[32];
		snprintf(buf,31,"%.1f hours", ((*itr).second->Quantity / 100)); // 100xp per hour
		cout << " (" << buf << ")";
	    }
	}
	cout << endl; 
	(*itr).second->Dump(level+1);	
    }

    if (SubNodes.size() > 0) { return ; }

    // only show the messages for the leaf nodes
    list<string>::iterator msgEntry = Notes.begin();
    for (; msgEntry != Notes.end(); ++msgEntry) {
	cout << indent << (*msgEntry) << endl; 
    }	
}

string Cost::SerializeJson(Cost *cost) {
    // I'm not super happy with the way this ended up - it seems like it could be clearer.
    // This function is called recursively.  It always returns a JSON array.  If the current
    // node (cost) has children then it will be an array of Costs (this class).  If the
    // current node has no children, it will return an array of strings - which are intended
    // to be the details of the cost.  IE, "10 Coal for 1 of Steel Blanks".

    if (cost == NULL) { return "[]"; }

    string retVal = "[ ";

    if (cost->SubNodes.size() < 1) {
	 bool addSep = false;
	 list<string>::iterator msgEntry = cost->Notes.begin();
	 for (; msgEntry != cost->Notes.end(); ++msgEntry) {
	     if (addSep) { retVal += ", "; } else { addSep = true; }
	     retVal += "\"" + *msgEntry + "\"";
	 }
    } else {
	bool addSep = false;

	EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();
	map<short, Cost*>::iterator itr = cost->SubNodes.begin();
	for (; itr != cost->SubNodes.end(); ++itr) {
	    Cost *child = (*itr).second;
	    
	    if (addSep) { retVal += ", "; } else { addSep = true; }
	    list<string> typeList = eTypeHelper->GetType(child->Type);
	    string childName = typeList.back();
	    
	    retVal += "{ \"Name\": \"" + childName + "\"";
	    retVal += ", \"Rank\": " + to_string(child->Rank);
	    retVal += ", \"Quantity\": " + to_string(child->Quantity);
	    
	    if (child->SubNodes.size() > 0) {
		retVal += ", \"Children\": ";
	    } else {
		retVal += ", \"Detail\": ";
	    }
	    retVal += SerializeJson(child);
	    retVal += " }";
	}
    }

    retVal += " ]";
    return retVal;
}
