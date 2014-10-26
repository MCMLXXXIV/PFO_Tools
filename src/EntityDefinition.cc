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



string EntityDefinition::Dump(const EntityDefinition &item) {
    // return Dump(item, "", -1.0);
    Dump_StdOut(item, "", -1.0);
    return "";
}

string EntityDefinition::Dump(const EntityDefinition &item, const char* indent, double quantity) {
    EntityTypeHelper *h = EntityTypeHelper::Instance();
    string retVal = indent;

    cout << item.Name << endl;

    if (strlen(indent) > 60) { return ""; }

    char buf[16];
    if (quantity < 0) {
	retVal += "         ";
    } else {
	if (h->QuantityIsWholeNumber(item.Type[0])) {
	    snprintf(buf, 15, "%5.f", quantity);
	} else {
	    snprintf(buf, 15, "%5.3f", quantity);
	}
	retVal += buf;
	retVal += " of ";
    }

    if (item.ProcessedSpreadsheetDefinition) {
	retVal += "*";
    } else {
	retVal += " ";
    }
    retVal += item.Name;
    retVal += "; Incr:";
    retVal += item.CreationIncrement;
    retVal += "; Type:";
    retVal += EntityTypeHelper::Instance()->ToIdString(item.Type);

    if (item.Requirements.size() == 0) {
	retVal += " (no requirements)\n";
	return retVal;
    } else {
	vector < list < LineItem* > >::const_iterator rankItr;
	for (rankItr = (item.Requirements).begin(); rankItr != item.Requirements.end(); ++rankItr) {
	    const list<LineItem*> *reqs = &(*rankItr);
	    list<LineItem*>::const_iterator itr;
	    for (itr = reqs->begin(); itr != reqs->end(); ++itr) {
		string newIndent = indent;
		newIndent += "    ";
		retVal += Dump(*((*itr)->Entity), newIndent.c_str(), (*itr)->Quantity); 
	    }
	}
    }
    retVal += "\n";
    return retVal;
}

void EntityDefinition::Dump_StdOut(const EntityDefinition &item, const char* indent, double quantity) {
    EntityTypeHelper *h = EntityTypeHelper::Instance();
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

    if (item.Requirements.size() == 0) {
	cout << " (no requirements)" << endl;
	return;
    } else {
	if (h->IsRanked(item.Type[0])) {
	    cout << "; " << item.Requirements.size() << " ranks" << endl;
	    vector < list < LineItem* > >::const_iterator rankItr;
	    int rank = 0;
	    for (rank = 0, rankItr = (item.Requirements).begin(); rankItr != item.Requirements.end(); ++rankItr, ++rank) {
		if (rank == 0) {
		    assert((*rankItr).size() == 0);
		    continue;
		}
		cout << indent << "  Rank " << rank << " of " << item.Name << ":" << endl;
		string newIndent = indent;
		newIndent += "    ";
		const list<LineItem*> *reqs = &(*rankItr);
		list<LineItem*>::const_iterator itr;
		for (itr = reqs->begin(); itr != reqs->end(); ++itr) {
		    if ((*itr)->Entity == &item) {
			snprintf(buf, 15, "%2.f", (*itr)->Quantity);
			cout << newIndent << processedFlag << item.Name << ", rank " << buf << endl;
		    } else {
			Dump_StdOut(*((*itr)->Entity), newIndent.c_str(), (*itr)->Quantity); 
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
		    Dump_StdOut(*((*itr)->Entity), newIndent.c_str(), (*itr)->Quantity); 
		}
	    }
	}
    }
    return;
}
