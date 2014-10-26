#ifndef ENTITYDEFINITION_H
#define ENTITYDEFINITION_H

#include <string>
#include <list>
#include <vector>
#include <iostream>

class LineItem;

using namespace std;

class EntityDefinition {

 public:
    // EntityDefinition();
    static string Dump(const EntityDefinition &item);

    string Name;
    bool ProcessedSpreadsheetDefinition;
    int CreationIncrement;
    short* Type; // will be an array of shorts

    vector < list < LineItem* > > Requirements;
    vector < list < LineItem* > > Provides;

 private:
    static string Dump(const EntityDefinition &item, const char* indent, double quantity);
    static void Dump_StdOut(const EntityDefinition &item, const char* indent, double quantity);
};

ostream &operator<<(ostream &os, const EntityDefinition &item);

#endif
