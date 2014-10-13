#ifndef ENTITYDEFINITION_H
#define ENTITYDEFINITION_H

#include <string>
#include <list>
#include <vector>

class LineItem;

using namespace std;

class EntityDefinition {

 public:
    // EntityDefinition();

    string Name;
    bool ProcessedSpreadsheetDefinition;
    int CreationIncrement;
    short* Type; // will be an array of shorts

    vector < list < LineItem* > > Requirements;
    vector < list < LineItem* > > Provides;
};

#endif