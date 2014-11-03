#include "LineItem.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

LineItem::LineItem(EntityDefinition* entity, double quantity) {
    Entity = entity;
    Quantity = quantity;
}


void LineItem::Dump() {
    printf("%5.2f of %s\n", Quantity, Entity->Name.c_str());
}

string LineItem::Describe(LineItem *parent) {
    string retVal;

    if (parent != NULL) {
	char buf[64];
	snprintf(buf, 63, "For %2.f of %-18s: ", parent->Quantity, parent->Entity->Name.c_str());
	retVal += buf;
    }

    if (EntityTypeHelper::Instance()->IsType(Entity->Type[0], "LogicOr")) {
	list<LineItem*> *reqs = &(Entity->Requirements.back());
	list<LineItem*>::iterator itr = reqs->begin();
	for (; itr != reqs->end(); ++itr) {
	    if (itr != reqs->begin()) {
		retVal += " or ";
	    }
	    char buf[64];
	    snprintf(buf, 63, "%2.f of %-12s", (*itr)->Quantity, (*itr)->Entity->Name.c_str());
	    retVal += buf;
	}
    } else {
	char buf[64];
	snprintf(buf, 63, "%5.2f of %s", Quantity, Entity->Name.c_str());
	retVal += buf;
    }
    return retVal;
}
