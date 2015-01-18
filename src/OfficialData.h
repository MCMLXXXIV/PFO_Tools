#ifndef OFFICIALDATA_H
#define OFFICIALDATA_H

#include "EntityDefinition.h"

#include <boost/algorithm/string.hpp>

class OfficialData;

typedef bool (OfficialData::*FileProcessor)(string,bool);

class OfficialData {
 public:
    static OfficialData* Instance();
    int ProcessSpreadsheetDir(string dirname);
    bool TestParseFile(string fn);
    EntityDefinition* GetEntity(string name);
    void SearchForItemsThatRequire(EntityDefinition* entity);
    vector<string> SearchForEntitiesMatchingStrings(const char*);
    void Dump();
    void ShowDataFileCoverage();

 private:
    OfficialData();

    bool ParseAndStoreCraftingRecipeFile(string fn, bool testRun);
    bool ParseAndStoreRefiningRecipeFile(string fn, bool testRun);
    bool ParseAndStoreSkillsAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreArmorAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreAttackAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreBonusesAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreCantripAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreDefensiveAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreExpendablesAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreFeatureAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreOrisonAdvancementFile(string fn, bool testRun);
    bool ParseAndStorePointsAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreProficienciesAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreReactiveAdvancementFile(string fn, bool testRun);
    bool ParseAndStoreUtilityAdvancementFile(string fn, bool testRun);

    bool ParseAndStoreFeatAchievements(string fn, bool testRun);
    bool ParseAndStoreCrowdforgedRecipeDataFile(string fn, bool testRun);

    bool ParseAndStoreRecipeFile(string fn, bool testRun, string subtype);
    bool ParseAndStoreProgressionFile(string fn, bool testRun, string eType);

    LineItem* ParseRequirementString(string reqStr, string entityName, string &errMsg);
    LineItem* BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName);

    //EntityDefinition* FindEntity(string type, string name);
    //EntityDefinition* FindEntity(list<string> fqName);

    bool StoreEntity(string fullyQualifiedName, EntityDefinition *entity);

    struct comp {
	bool operator() (const string& lhs, const string& rhs) const {
	    // http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
	    return boost::ilexicographical_compare(lhs, rhs);
	}
    };

    map< string, EntityDefinition*, comp> Entities;
    map< string, FileProcessor > FileProcessorMap;

    static OfficialData* m_pInstance;
};

#endif
