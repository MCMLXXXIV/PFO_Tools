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

    if (EntityTypeHelper::Instance()->IsRanked((entity->Type)[0])) {
	if (childNode->Sum < item->Quantity) {
	    childNode->Sum = item->Quantity;
	}
    } else if (EntityTypeHelper::Instance()->IsUniversal((entity->Type)[0])) {
	childNode->Sum = item->Quantity;
    } else {
	childNode->Sum += item->Quantity;
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

    map<short, Cost*>::iterator itr = SubNodes.begin();
    for (; itr != SubNodes.end(); ++itr) {
	short *type = (*itr).second->Type;
	list<string> typeList = eTypeHelper->GetType(type);
	cout << indent << typeList.back() << ":";

	// only show the summed quantity for Time and ExperiencePoints
	if (level == 1 && (eTypeHelper->IsType(type[0],"Time") || eTypeHelper->IsType(type[0],"ExperiencePoint"))) {
	    cout << " " << (*itr).second->Sum;
	    if (eTypeHelper->IsType(type[0], "ExperiencePoint")) {
		char buf[32];
		snprintf(buf,31,"%.1f hours", ((*itr).second->Sum / 100)); // 100xp per hour
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
