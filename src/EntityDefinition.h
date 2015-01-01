#ifndef ENTITYDEFINITION_H
#define ENTITYDEFINITION_H

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <set>
#include <map>

class LineItem;

using namespace std;

class EntityDefinition {

 public:
    EntityDefinition();
    static string SerializeJson(EntityDefinition *entity);
    static string Dump(const EntityDefinition &item, double quantity);
    bool HasRequirement(EntityDefinition* targetEntity, set<EntityDefinition*> &searched);

    string Name;
    bool ProcessedSpreadsheetDefinition;
    int CreationIncrement;
    short* Type; // will be an array of shorts

    vector < list < LineItem* > > Requirements;
    vector < list < LineItem* > > Provides;

 private:
    static string Dump(const EntityDefinition &item, const char* indent, double quantity) { return ""; };
    static void Dump_StdOut(const EntityDefinition &item, const char* indent, double quantity, map<const EntityDefinition*, int> &shown);
};

ostream &operator<<(ostream &os, const EntityDefinition &item);

#endif
