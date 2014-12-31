#include "LineItem.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

#include <cstring>

LineItem::LineItem() : Entity(NULL), Rank(0), Quantity(0) {}

LineItem::LineItem(EntityDefinition* entity, int rank, double quantity) :
    Entity(entity), Rank(rank), Quantity(quantity) {}


void LineItem::Dump() {
    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();
    char qtyStr[16];
    memset(qtyStr, '\0', 16);
    char rankStr[32];
    memset(rankStr, '\0', 32);

    if (eTypeHelper->IsRanked(Entity->Type[0])) {
	snprintf(rankStr, 31, ", rank %d", Rank);
    } else {
	if (eTypeHelper->QuantityIsWholeNumber(Entity->Type[0])) {
	    snprintf(qtyStr, 15, "%1.f of ", Quantity);
	} else {
	    snprintf(qtyStr, 15, "%.3f of ", Quantity);
	}
	if (eTypeHelper->IsType(Entity->Type[0], "Item")) {
	    // Rank is unsigned for now - but still leaving the ternary operator here
	    snprintf(rankStr, 31, " %c%d", (Rank < 0 ? '-' : '+'), Rank);
	}
    }

    printf("%6s%s%s\n", qtyStr, Entity->Name.c_str(), rankStr);
}

string LineItem::ToString() {
    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    char qtyStr[16];
    memset(qtyStr, '\0', 16);
    char rankStr[32];
    memset(rankStr, '\0', 32);

    if (eTypeHelper->IsRanked(Entity->Type[0])) {
	snprintf(rankStr, 31, ", rank %d", Rank);
    } else {
	if (eTypeHelper->QuantityIsWholeNumber(Entity->Type[0])) {
	    snprintf(qtyStr, 15, "%1.f of ", Quantity);
	} else {
	    snprintf(qtyStr, 15, "%.3f of ", Quantity);
	}
	if (eTypeHelper->IsType(Entity->Type[0], "Item")) {
	    // Rank is unsigned for now - but still leaving the ternary operator here
	    snprintf(rankStr, 31, " %c%d", (Rank < 0 ? '-' : '+'), Rank);
	}
    }

    char completeStr[255];
    memset(completeStr, '\0', 255);
    snprintf(completeStr, 254, "%6s%s%s", qtyStr, Entity->Name.c_str(), rankStr);

    return string(completeStr);
}

string LineItem::Describe(LineItem *parent) {
    string retVal;

    if (parent != NULL) {
	retVal += "For ";
	retVal += parent->ToString();
	retVal += ": ";
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
	retVal += this->ToString();
    }
    return retVal;
}

string LineItem::SerializeJson(LineItem *lineItem) {
    if (lineItem == NULL) { return "{}"; }

    string retVal = "{ \"Name\": ";
    retVal += "\"" + lineItem->Entity->Name + "\"";
    retVal += ", \"Rank\": ";
    retVal += to_string(lineItem->Rank);
    retVal += ", \"Quantity\": ";
    retVal += to_string(lineItem->Quantity);
    retVal += " }";

    return retVal;
}
