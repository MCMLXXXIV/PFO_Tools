#ifndef OFFICIALDATA_H
#define OFFICIALDATA_H

#include <map>
#include <string>

#include "EntityDefinition.h"

class OfficialData;

typedef bool (OfficialData::*FileProcessor)(string);

class OfficialData {
 public:
    static OfficialData* Instance();
    int ProcessSpreadsheetDir(string dirname);
    EntityDefinition* GetEntity(int dummy, string name);
    void Dump();
    void SearchForItemsThatRequire(EntityDefinition* entity);
    
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
    
    bool ParseAndStoreRecipeFile(string fn, string subtype);
    bool ParseAndStoreProgressionFile(string fn, string eType);

    LineItem* ParseRequirementString(string reqStr, string entityName, string &errMsg);
    LineItem* BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName);

    //EntityDefinition* FindEntity(string type, string name);
    //EntityDefinition* FindEntity(list<string> fqName);

    bool StoreEntity(string fullyQualifiedName, EntityDefinition *entity);

    map< string, EntityDefinition* > EntitiesV2;
    map< string, FileProcessor > FileProcessorMap;

    static OfficialData* m_pInstance;
};

#endif
