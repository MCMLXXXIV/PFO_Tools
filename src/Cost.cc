#include "Cost.h"
#include "LineItem.h"
#include "CostNode.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

#include <iostream>

using namespace std;

//void Cost::Add(LineItem *item) {
//    TempCostList.push_back(new LineItem(item->Entity, item->Quantity));
//}

void Cost::Add(LineItem *item) {
    if (item == NULL) { return; }

    map<short, CostNode*> *costNodes = &CostList;
    short *id = item->Entity->Type;
    while (*id != 0) {
	CostNode* costNode = (*costNodes)[*id];
	if (costNode == NULL) {
	    costNode = new CostNode();
	    (*costNodes)[*id] = costNode;
	}
	costNode->Sum += item->Quantity;
	
	++id;
	costNodes = &(costNode->SubNodes);
    }
}

void Cost::Dump() {
    map<short, CostNode*>::iterator itr = CostList.begin();
    for (; itr != CostList.end(); ++itr) {
	short id[2];
	id[0] = (*itr).first;
	id[1] = 0;
	list<string> type = EntityTypeHelper::Instance()->GetType(id);
	cout << type.front() << ": " << (*itr).second->Sum << endl;
    }

}
