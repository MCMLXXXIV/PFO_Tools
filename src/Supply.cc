#include "Supply.h"
#include "EntityTypeHelper.h"
#include "EntityDefinition.h"
#include "LineItem.h"
#include "OfficialData.h"

// TODO: test that the item name is case insensitive

bool Supply::Withdrawal(LineItem *item) {
    // return true if the item is completely satisfied from the supply, false otherwise
    //
    // item may be modified - the Quantity may be reduced if we satisfy some from the supply

    // rank zero of a feat is invariably free - I think it's default so it should be considered
    // already in the bank.

    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    if (item->Rank == 0 && eTypeHelper->IsRanked(item->Entity->Type[0])) {
	return true;
    }

    string itemId = eTypeHelper->ToIdString(item->Entity->Type);
    auto itr = Items.find(itemId);
    if (itr == Items.end()) {
	return false;
    }

    if (eTypeHelper->IsRanked(item->Entity->Type[0])) {
	if (item->Rank > Items[itemId][0]->Rank) {
	    return false;
	} else {
	    return true;
	}
    } else if (eTypeHelper->IsUniversal(item->Entity->Type[0])) {
	// AbilityScore and AchievementPoints
	// note that Achievement and Feat are also Universal but they are handled above, in the
	// IsRanked clause
	double diff = item->Quantity - (Items[itemId][0])->Quantity;
	if (diff <= 0.0) {
	    item->Quantity = 0.0;
	    return true;
	} else {
	    item->Quantity = diff;
	    return false;
	}
    } else {
	// bank will probably never store time - but it may store xp as a starting condition
	// ie, we will never add xp to the bank during planning but we may start with some
	// xp in the bank.  Still - for the most part, we will only be handling items here.
	LineItem *bankItem = NULL;
	if (eTypeHelper->IsType(item->Entity->Type[0], "Item")) {
	    if (Items[itemId].size() > item->Rank) {
		// we don't have any of that rank
		return false;
	    }
	    if (Items[itemId][item->Rank] == NULL) {
		return false;
	    }
	    bankItem = Items[itemId][item->Rank];
	} else {
	    bankItem = Items[itemId][0];
	}
	double diff = item->Quantity - bankItem->Quantity;
	if (diff <= 0.0) {
	    // had more in the bank than we needed
	    bankItem->Quantity -= item->Quantity;
	    item->Quantity = 0.0;
	    return true;
	} else {
	    // the withdrawer wanted more than we have currently
	    bankItem->Quantity = 0.0;
	    item->Quantity = diff;
	    return false;
	}
    }

    bool shouldNeverGetHere = true;
    assert(shouldNeverGetHere == false);
    return false;
}

void Supply::Deposit(LineItem *item) {
    EntityTypeHelper *eTypeHelper = EntityTypeHelper::Instance();

    // here is a little misplaced debugging - I *think* that if a thing is not ranked and is not an item
    // think Rank will always be zero
    if (eTypeHelper->IsRanked(item->Entity->Type[0]) == false) {
	if (eTypeHelper->IsType(item->Entity->Type[0], "Item") == false) {
	    if (item->Rank != 0) {
		cerr << "ERROR: this isn't ranked and it's not an item - Rank should be zero; " << item->ToString() << endl;
		bool itemIsntRankedAndNotAnItemSoRankIsZero = false;
		assert(itemIsntRankedAndNotAnItemSoRankIsZero);
	    }
	}
    }

    // Universal entities have only one entry in the list if lineitems for the entity.
    // All others will have a number of entries equal to the highest rank plus one.
    // Note that the way things are written right now, the only non-universal entities
    // with Ranks greater than zero are items.
    unsigned entryIndex = 0;
    if (eTypeHelper->IsUniversal(item->Entity->Type[0]) == false) {
	entryIndex = item->Rank;
    }

    string itemId = eTypeHelper->ToIdString(item->Entity->Type);
    auto itr = Items.find(itemId);
    if (itr == Items.end()) {
	vector<LineItem*> listOfItems;
	Items[itemId] = listOfItems;
	while (Items[itemId].size() <= entryIndex) {
	    Items[itemId].push_back(NULL);
	}
	Items[itemId][entryIndex] = new LineItem(item->Entity, item->Rank, item->Quantity);
	return;
    }

    if (Items[itemId][entryIndex] == NULL) {
	Items[itemId][entryIndex] = new LineItem(item->Entity, item->Rank, item->Quantity);
	return;
    }

    LineItem *bankItem = Items[itemId][entryIndex];

    if (eTypeHelper->IsRanked(bankItem->Entity->Type[0])) {
	if (item->Rank > bankItem->Rank) {
	    bankItem->Rank = item->Rank;
	}
	return;
    } else if (eTypeHelper->IsUniversal((bankItem->Entity->Type)[0])) {
	if (item->Quantity > bankItem->Quantity) {
	    bankItem->Quantity = item->Quantity;
	}
	return;
    }

    bankItem->Quantity += item->Quantity;
    return;
}

void Supply::Dump() {
    auto itr = Items.begin();
    for (; itr != Items.end(); ++itr) {
	auto itemEntry = (*itr).second.begin();
	for (; itemEntry != (*itr).second.end(); ++itemEntry) {
	    LineItem *item = *itemEntry;
	    if (item == NULL) {
		continue;
	    }
	    if (EntityTypeHelper::Instance()->IsType(item->Entity->Type[0], "AbilityScore")) {
		if (item->Quantity <= 10.0) {
		    continue;
		}
	    }
	    item->Dump();
	}
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
	retVal->Deposit(new LineItem(abiEntity, 0, 10.0));
    }
    
    return retVal;
}

string Supply::SerializeJson(Supply *supply) {
    if (supply == NULL) { return "[]"; }

    string retVal = "[";
    bool addSep = false;
    auto itr = supply->Items.begin();
    for (; itr != supply->Items.end(); ++itr) {
	auto itemEntry = (*itr).second.begin();
	for (; itemEntry != (*itr).second.end(); ++itemEntry) {
	    LineItem *item = *itemEntry;
	    if (item == NULL) {
		continue;
	    }
	    if (addSep) { retVal += ", "; } else { addSep = true; }
	    retVal += LineItem::SerializeJson(item);
	}
    }
    retVal += " ]";
    return retVal;
}
