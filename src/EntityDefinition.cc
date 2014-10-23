#include "EntityDefinition.h"
#include "EntityTypeHelper.h"

#include <cstring>

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
