#include "Supply.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "LineItem.h"

bool Supply::Has(EntityDefinition *entity) {
    map<string, LineItem*>::iterator itr = Items.find(EntityTypeHelper::Instance()->ToIdString(entity->Type));
    return (itr != Items.end());
}

double Supply::Withdrawal(LineItem *item) {
    string itemId = EntityTypeHelper::Instance()->ToIdString(item->Entity->Type);
    map<string, LineItem*>::iterator itr = Items.find(itemId);
    if (itr == Items.end()) {
	return item->Quantity;
    }

    if (EntityTypeHelper::Instance()->IsRanked(item->Entity->Type[0])) {
	if (item->Quantity > (Items[itemId])->Quantity) {
	    return item->Quantity;
	} else {
	    return 0.0;
	}
    } else if (EntityTypeHelper::Instance()->IsUniversal(item->Entity->Type[0])) {
	double diff = item->Quantity - (Items[itemId])->Quantity;
	if (diff <= 0.0) {
	    return 0.0;
	} else {
	    return diff;
	}
    } else {
	// bank will probably never store time - but it may store xp as a starting condition
	// ie, we will never add xp to the bank during planning but we may start with some
	// xp in the bank.  Still - for the most part, we will only be handling items here.
	LineItem *bankItem = Items[itemId];
	double diff = item->Quantity - bankItem->Quantity;
	if (diff <= 0.0) {
	    // had more in the bank than we needed
	    bankItem->Quantity -= item->Quantity;
	    return 0.0;
	} else {
	    // the withdrawer wanted more than we have currently
	    bankItem->Quantity = 0.0;
	    return diff;
	}
    }
    return 0.0;
}

void Supply::Deposit(LineItem *item) {
    LineItem *bankItem = NULL;
    string itemId = EntityTypeHelper::Instance()->ToIdString(item->Entity->Type);
    map<string, LineItem*>::iterator itr = Items.find(itemId);
    if (itr != Items.end()) {
	bankItem = (*itr).second;
    } else {
	bankItem = new LineItem(item->Entity, item->Quantity);
	Items[itemId] = bankItem;
	return;
    }

    if (EntityTypeHelper::Instance()->IsRanked(bankItem->Entity->Type[0])) {
	if (item->Quantity > bankItem->Quantity) {
	    bankItem->Quantity = item->Quantity;
	}
	return;
    }

    // items and achievement points here - all others are Ranked or
    // never produced (eg, time, xp) and so never deposited
    bankItem->Quantity += item->Quantity;
    return;
}

void Supply::Dump() {
    map<string, LineItem*>::iterator itr = Items.begin();
    for (; itr != Items.end(); ++itr) {
	(*itr).second->Dump();
    }
}
