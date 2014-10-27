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
    Dump("");
    if (NonAggregateCosts.size() > 0) {
	cout << endl << "Unaggregated costs:" << endl;
	list<string>::iterator itr;
	for (itr = NonAggregateCosts.begin(); itr != NonAggregateCosts.end(); ++itr) {
	    cout << "    " << *itr << endl;
	}
    }
}

void Cost::Dump(string indent) {

    map<short, Cost*>::iterator itr = SubNodes.begin();
    for (; itr != SubNodes.end(); ++itr) {
	list<string> type = EntityTypeHelper::Instance()->GetType((*itr).second->Type);
	cout << indent << type.back() << ": " << (*itr).second->Sum << endl; 
	(*itr).second->Dump(indent + "    ");	
    }
    if (SubNodes.size() > 0) { return ; }
    list<string>::iterator msgEntry = Notes.begin();
    for (; msgEntry != Notes.end(); ++msgEntry) {
	cout << indent << (*msgEntry) << endl; 
    }	
}
