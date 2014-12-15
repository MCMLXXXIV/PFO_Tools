#include "Supply.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "LineItem.h"
#include "OfficialData.h"

double Supply::Withdrawal(LineItem *item) {
    // rank zero of a feat is invariably free - I think it's default so it should be considered
    // already in the bank.  This is handled w/o any extra code because of the way we handle all
    // requests for zero of something - just return zero which means "zero more are required"

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
    } else if (EntityTypeHelper::Instance()->IsUniversal((bankItem->Entity->Type)[0])) {
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
	if (EntityTypeHelper::Instance()->IsType((*itr).second->Entity->Type[0], "AbilityScore")) {
	    if ((*itr).second->Quantity <= 10.0) {
		continue;
	    }
	}
	(*itr).second->Dump();

    }
}

Supply* Supply::Deserialize(const char *buf) {

    Supply *retVal = new Supply();
    
    // add default stats to bank
    //
    // N:AbilityScore.Constitution; P:0; C:0; ID:4.4
    // N:AbilityScore.Dexterity; P:0; C:0; ID:4.3
    // N:AbilityScore.Intelligence; P:0; C:0; ID:4.1
    // N:AbilityScore.Personality; P:0; C:0; ID:4.2
    // N:AbilityScore.Strength; P:0; C:0; ID:4.5
    // N:AbilityScore.Wisdom; P:0; C:0; ID:4.6
    set<string> abilityNames = {
	"Constitution",
	"Dexterity",
	"Intelligence",
	"Personality",
	"Strength",
	"Wisdom"
    };
    
    set<string>::iterator abilityItem;
    for (abilityItem = abilityNames.begin(); abilityItem != abilityNames.end(); ++abilityItem) {
	string entityName = "AbilityScore." + (*abilityItem);
	EntityDefinition *abiEntity = OfficialData::Instance()->GetEntity(entityName);
	assert(abiEntity != NULL);
	retVal->Deposit(new LineItem(abiEntity, 10.0));
    }
    
    return retVal;
}

string Supply::SerializeJson(Supply *supply) {
    if (supply == NULL) { return "[]"; }

    string retVal = "[";
    bool addSep = false;
    map<string, LineItem*>::iterator itr = supply->Items.begin();
    for (; itr != supply->Items.end(); ++itr) {
	if (addSep) { retVal += ", "; } else { addSep = true; }
	retVal += LineItem::SerializeJson((*itr).second);
    }
    retVal += " ]";
    return retVal;
}
