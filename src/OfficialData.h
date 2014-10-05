#ifndef OFFICIALDATA_H
#define OFFICIALDATA_H

#include <map>
#include <string>

#include "EntityDefinition.h"

class OfficialData;

typedef bool (OfficialData::*FileProcessor)(string);

class OfficialData {
 public:
    OfficialData();
    int ProcessSpreadsheetDir(string dirname);
    
 private:
    bool ParseAndStoreCraftingRecipeFile(string fn);
    bool ParseAndStoreRefiningRecipeFile(string fn);

    bool ParseAndStoreRecipeFile(string fn, EntitySubType subtype);

    map< string, EntityDefinition* > Entities;
    map< string, FileProcessor > FileProcessorMap;
    
};

#endif
