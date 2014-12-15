#include "LineItem.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

LineItem::LineItem(EntityDefinition* entity, double quantity) {
    Entity = entity;
    Quantity = quantity;
}


void LineItem::Dump() {
    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    char buf[16];
    if (eTypeHelper->QuantityIsWholeNumber(Entity->Type[0])) {
	snprintf(buf, 15, "%1.f", Quantity);
    } else {
	snprintf(buf, 15, "%.3f", Quantity);
    }

    printf("%6s of %s\n", buf, Entity->Name.c_str());
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

string LineItem::SerializeJson(LineItem *lineItem) {
    if (lineItem == NULL) { return "{}"; }

    string retVal = "{ \"Name\": ";
    retVal += "\"" + lineItem->Entity->Name + "\"";
    retVal += ", \"Quantity\": ";
    retVal += to_string(lineItem->Quantity);
    retVal += " }";

    return retVal;
}
