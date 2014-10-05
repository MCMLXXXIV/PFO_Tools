#ifndef ENTITYDEFINITION_H
#define ENTITYDEFINITION_H

#include <string>
#include <list>

class RequirementNode;
class ProvidesNode;

using namespace std;

enum EntityType { Item, Recipe, Achievement, Feat, Skill };
enum EntitySubType { None, Refine, Craft };

class EntityDefinition {

 public:
    // EntityDefinition();

    string Name;
    bool ProcessedSpreadsheetDefinition;
    EntityType Type;
    EntitySubType SubType;
    int CreationIncrement;
    bool Universal;

    list< RequirementNode* > Requirements;
    list< ProvidesNode* > Provides;

};

#endif
