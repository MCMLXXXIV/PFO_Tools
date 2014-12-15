#include "EntityDefinition.h"
#include "EntityTypeHelper.h"
#include "LineItem.h"

#include <cstring>
#include <cassert>

ostream &operator<<(ostream &os, const EntityDefinition &item) {
    EntityTypeHelper *h = EntityTypeHelper::Instance();
    
    list<string> parts = h->GetType(item.Type);
    string idString = "";
    for (list<string>::iterator itr = parts.begin(); itr != parts.end(); ++itr) {
	if (idString.size() > 0) {
	    idString += ".";
	}
	idString += *itr;
    }

    os << "N:" << idString << "; P:" << item.ProcessedSpreadsheetDefinition 
       << "; C:" << item.CreationIncrement << "; ID:" << h->ToIdString(item.Type);
    return os;
}

string EntityDefinition::SerializeJson(EntityDefinition *entity) {
    if (entity == NULL) { return ""; }
    string retVal = "{ \"Name\": ";
    retVal += "\"" + entity->Name + "\"";
    retVal += " }";

    return retVal;
}

string EntityDefinition::Dump(const EntityDefinition &item, double quantity) {
    // return Dump(item, "", -1.0);
    map<const EntityDefinition*, int> shown;
    Dump_StdOut(item, "", quantity, shown);
    return "";
}

void EntityDefinition::Dump_StdOut(const EntityDefinition &item, const char* indent, double quantity,
				   map<const EntityDefinition*, int> &shown) {
    EntityTypeHelper *h = EntityTypeHelper::Instance();
    bool isRanked = h->IsRanked(item.Type[0]);
    bool isUniversal = h->IsUniversal(item.Type[0]);
    int stopAtRank = -1;
    bool preventRecurse = false;

    map<const EntityDefinition*, int>::iterator shownItr = shown.find(&item);
    if (isRanked) {
	int rank = (int)quantity;
	if (shownItr == shown.end()) {
	    shown[&item] = (int)quantity;
	} else {
	    if ((*shownItr).second < rank) {
		stopAtRank = (*shownItr).second;
		shown[&item] = rank;
	    } else {
		preventRecurse = true;
	    }
	}
    } else {
	if (shownItr != shown.end()) {
	    if (!isUniversal) {
		preventRecurse = true;
	    }
	}
	shown[&item] = 0;
    }
    cout << indent;

    assert(strlen(indent) < 61);
    // if (strlen(indent) > 60) { return; }
    string processedFlag = " ";

    if (item.ProcessedSpreadsheetDefinition) {
	processedFlag = "*";
    }

    cout << processedFlag << item.Name;

    char buf[16];
    if (quantity < 0) {
	cout << "            ";
    } else {
	if (h->QuantityIsWholeNumber(item.Type[0])) {
	    snprintf(buf, 15, "%5.f", quantity);
	} else {
	    snprintf(buf, 15, "%5.3f", quantity);
	}
	cout << "; qty: " << buf;
    }
    cout << "; Incr:" << item.CreationIncrement << "; Type:" << EntityTypeHelper::Instance()->ToIdString(item.Type);

    if (preventRecurse) {
	if (h->IsType(item.Type[0], "Item")) {
	    cout << " (see requirements above)";
	}
	cout << endl;
	return;
    }

    if (item.Requirements.size() == 0) {
	cout << " (no requirements)" << endl;
	return;
    } else {
	if (isRanked) {
	    cout << "; " << item.Requirements.size() << " ranks" << endl;
	    for (int rank = (int)quantity; rank > stopAtRank; --rank) {
		if (rank >= (int)item.Requirements.size()) {
		    continue;
		}
		const list<LineItem*> *reqs = &(item.Requirements[rank]);
		if (rank == 0) {
		    assert(reqs->size() == 0);
		    continue;
		}

		cout << indent << "  Rank " << rank << " of " << item.Name << ":" << endl;
		string newIndent = indent;
		newIndent += "    ";
		list<LineItem*>::const_iterator itr;
		for (itr = reqs->begin(); itr != reqs->end(); ++itr) {
		    if ((*itr)->Entity == &item) {
			snprintf(buf, 15, "%2.f", (*itr)->Quantity);
			cout << newIndent << processedFlag << item.Name << ", rank " << buf << endl;
		    } else {
			Dump_StdOut(*((*itr)->Entity), newIndent.c_str(), (*itr)->Quantity, shown); 
		    }
		}
	    }
	} else {
	    if (item.Requirements.size() != 1) {
		cout << endl << "ERROR: this item isn't ranked but it has " << item.Requirements.size()
		     << " lists of requirements.  It should only have two and that first one should be empty"
		     << endl;
	    }
	    assert(item.Requirements.size() == 1);
	    cout << endl;
	    string newIndent = indent;
	    newIndent += "    ";
	    const list<LineItem*> *reqs = &((item.Requirements).back());
	    list<LineItem*>::const_iterator itr;
	    for (itr = reqs->begin(); itr != reqs->end(); ++itr) {
		assert((*itr)->Entity != &item);
		if ((*itr)->Entity == &item) {
		    snprintf(buf, 15, "%2.f", (*itr)->Quantity);
		    cout << newIndent << processedFlag << item.Name << ", rank " << buf << endl;
		} else {
		    Dump_StdOut(*((*itr)->Entity), newIndent.c_str(), (*itr)->Quantity, shown); 
		}
	    }
	}
    }
    return;
}


bool EntityDefinition::HasRequirement(EntityDefinition* targetEntity, set<EntityDefinition*> &searched) {
    if (Requirements.size() < 1) {
	return false;
    }
    
    if (searched.count(this)) {
	return false;
    } else {
	searched.insert(this);
    }

    vector < list < LineItem* > >::iterator reqListEntry = Requirements.begin();
    for (; reqListEntry != Requirements.end(); ++reqListEntry) {
	list < LineItem* >::iterator itr;
	for (itr = (*reqListEntry).begin(); itr != (*reqListEntry).end(); ++itr) {
	    LineItem *req = *itr;
	    if (req->Entity == NULL) { continue; }
	    if (req->Entity == this) { continue; }
	    if (req->Entity == targetEntity) { return true; }
	    if (req->Entity->HasRequirement(targetEntity, searched)) { return true; }
	}
    }

    return false;
}
