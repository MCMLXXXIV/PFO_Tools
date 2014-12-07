#ifndef OFFICIALDATA_H
#define OFFICIALDATA_H

#include <map>
#include <string>
#include <strings.h>
#include "EntityDefinition.h"

class OfficialData;

typedef bool (OfficialData::*FileProcessor)(string);

class OfficialData {
 public:
    static OfficialData* Instance();
    int ProcessSpreadsheetDir(string dirname);
    EntityDefinition* GetEntity(string name);
    void Dump();
    void SearchForItemsThatRequire(EntityDefinition* entity);
    vector<string> SearchForEntitiesMatchingStrings(const char*);

    static bool ParseProgressionFile(string fn);

 private:
    OfficialData();

    bool ParseAndStoreCraftingRecipeFile(string fn);
    bool ParseAndStoreRefiningRecipeFile(string fn);
    bool ParseAndStoreSkillsAdvancementFile(string fn);
    bool ParseAndStoreArmorAdvancementFile(string fn);
    bool ParseAndStoreAttackAdvancementFile(string fn);
    bool ParseAndStoreBonusesAdvancementFile(string fn);
    bool ParseAndStoreCantripAdvancementFile(string fn);
    bool ParseAndStoreDefensiveAdvancementFile(string fn);
    bool ParseAndStoreExpendablesAdvancementFile(string fn);
    bool ParseAndStoreFeatureAdvancementFile(string fn);
    bool ParseAndStoreOrisonAdvancementFile(string fn);
    bool ParseAndStorePointsAdvancementFile(string fn);
    bool ParseAndStoreProficienciesAdvancementFile(string fn);
    bool ParseAndStoreReactiveAdvancementFile(string fn);
    bool ParseAndStoreUtilityAdvancementFile(string fn);

    bool ParseAndStoreFeatAchievements(string fn);
    bool ParseAndStoreCrowdforgedRecipeDataFile(string fn);

    bool ParseAndStoreRecipeFile(string fn, string subtype);
    bool ParseAndStoreProgressionFile(string fn, string eType);

    LineItem* ParseRequirementString(string reqStr, string entityName, string &errMsg);
    LineItem* BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName);

    //EntityDefinition* FindEntity(string type, string name);
    //EntityDefinition* FindEntity(list<string> fqName);

    bool StoreEntity(string fullyQualifiedName, EntityDefinition *entity);

    struct comp {
	bool operator() (const string& lhs, const string& rhs) const {
	    // http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
	    // stack overflow tells me that there isn't a good way to do case-insensitive 
	    // comparisons except to use boost::iequals().  I haven't included boost yet
	    // so I'm not going to do that yet.  If on windows you could use stricmp and on
	    // POSIX systems you could use strcasecmp - and that's the route I'm taking here
	    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
	}
    };

    map< string, EntityDefinition*, comp> Entities;
    map< string, FileProcessor > FileProcessorMap;

    static OfficialData* m_pInstance;
};

#endif
