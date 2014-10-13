#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "OfficialData.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"
#include "LineItem.h"
#include "Utils.h"

using namespace std;

OfficialData::OfficialData() {
    FileProcessorMap["Recipes (Crafting).csv"] = &OfficialData::ParseAndStoreCraftingRecipeFile;
    FileProcessorMap["Recipes (Refining).csv"] = &OfficialData::ParseAndStoreRefiningRecipeFile;
}

EntityDefinition* OfficialData::GetEntity(string name) {
    map< string, EntityDefinition* >::iterator entityMapEntry;
    entityMapEntry = Entities.find(name);
    if (entityMapEntry == Entities.end()) {
	return NULL;
    } else {
	return (*entityMapEntry).second;
    }
}

int OfficialData::ProcessSpreadsheetDir(string directoryName) {
    DIR *dp;
    struct dirent *dirp;

    dp = opendir(directoryName.c_str());
    if (dp == NULL) {
	cout << "ERROR(" << errno << ") opening " << directoryName << endl;
	return errno;
    }

    while ((dirp = readdir(dp))) {
	string filepath;
	struct stat filestat;


	string fname = dirp->d_name;
	filepath = directoryName + "/" + fname;

	if (stat( filepath.c_str(), &filestat)) continue;
	if (S_ISDIR( filestat.st_mode)) continue;

	if (FileProcessorMap[fname]) {
	    (this->*FileProcessorMap[fname])(filepath);
	} else {
	    cout << "no handler for " << fname << endl;
	}	
    }
    closedir(dp);

    cout << "Done processing spreadsheet data; have " << Entities.size() << " entities" << endl;

    return 0;
}

bool OfficialData::ParseAndStoreCraftingRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Craft");
}

bool OfficialData::ParseAndStoreRefiningRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Refine");
}

bool OfficialData::ParseAndStoreRecipeFile(string fn, string subtype) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    cout << "+++ RUNNING: OfficialData::ParseAndStoreRecipeFile(" << fn << ")" << endl;

    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    while(getline(fin, line)) {
	// this is kind of ugly as the header line is incorrect at this time.  EG, it says:
	// |Component 3 and Number|  Achievement Type|Base Crafting Seconds|Last Updated
	// but a given row will have:
	// |Golden Crystal      |1|||Common          |2400                 |9-18-14

	// so for now, knowledge of the row format will be hardcoded here
	// fieldNum  Name                   Example
	//     0     Recipe                 Recipe
	//     1     Name                   Apprentice's Sunrod
	//     2     ReqSkill               Alchemist
	//     3     SkillRankReq           0
	//     4     Tier                   1
	//     5     Component1Name         Weak Luminous Extract
	//     6     Component1Qty          15
	//     7     Component2Name         Weak Acidic Extract
	//     8     Component2Qty          2
	//     9     Component3Name         Copper Bar
	//    10     Component3Qty          1
	//    11     EMPTY (comp4?)
	//    12     EMPTY (comp4?)
	//    13     Achievement Type       Common
	//    14     Base Crafting Seconds  3000
	//    15     Last Updated           9-18-14

	++line_num;

	// skip the first line - its a header line
	if (line_num < 2) continue;

	stringstream linestream(line);
	string val;
	vector<string> fields;
	while (getline(linestream, val, '|')) {
	    fields.push_back(val);
	}
	if (fields.size() != 16) {
	    cout << "WARNING: bad line in " << fn << ":" << line_num << "; doesn't have 16 fields - has " << fields.size() << ": " << line << endl;
	    continue;
	}
	if (fields[1].size() < 1) {
	    cout << "WARNING: bad line in " << fn << ":" << line_num << "; empty name; " << line << endl;
	    continue;
	}

	string *name = new string(fields[1]);
	int rankInName;
	char *namePart;
	if (Utils::RankInName(name->c_str(), &namePart, rankInName)) {
	    // if (regex_search(name, " \+[0-9]+$")) {
	    // cout << endl << namePart << "; Rank " << rankInName << " (from [" << *name << "])" << endl;
	    delete name;
	    name = new string(namePart);
	    delete namePart;
	    // right now we aren't storing entities for +1 and greater items
	    // there is much we would have to add to support this - right now for ranked
	    // items there is no other Quantity, so the Quantity in the LineItem does double
	    // duty for ranks and actual quantities.  But when we add Quantities of Ranked items
	    // (like 5 x "Steel Wire +1") then we will have to revamp the LineItem class.
	    if (rankInName > 0) {
		delete name;
		continue;
	    }		
	}

	if (*name == "Yew Stave") {
	    cout << endl << "Yew Stave Entity" << endl;
	}

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	map< string, EntityDefinition* >::iterator entityMapEntry;
	EntityDefinition *entity;
	entityMapEntry = Entities.find(*name);
	if (entityMapEntry != Entities.end()) {
	    entity = (*entityMapEntry).second;

	    // only add a Requirements and Provides list if we process the Entity from the spreadsheet
	    if (entity->Requirements.size() < 1) {
		entity->Requirements.push_back(*(new list<LineItem*>));
		entity->Provides.push_back(*(new list<LineItem*>));
	    }
	    assert(entity->ProcessedSpreadsheetDefinition == false);
	    cout << ".";
	} else {
	    cout << "+";
	    // I could (should?) check that these fields were set correctly the first time - but lets call that a todo
	    entity = new EntityDefinition();
	    entity->Name = *name;

	    list<string> typeFields;
	    typeFields.push_back("Item");
	    typeFields.push_back(*name);
	    entity->Type = typeHelper->GetType(typeFields);

	    // right now, Items aren't ranked - but we could extend this later to have a +1 item be rank two, etc
	    // still, we need to create the requirements and provides lists for the single rank
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));

	    Entities[*name] = entity;
	}
	// TODO FIXME TEST
	// soon I'll add code to consume the secondary, crowd sourced, table that give the yield
	// for refining stuff - for now, in order to do some testing of the goal solution code,
	// I'm going to make it a random number
	if (subtype == "Refine") {
	    entity->CreationIncrement = (rand() % 4) + 1;
	} else {
	    entity->CreationIncrement = 1;
	}
	entity->ProcessedSpreadsheetDefinition = true;
	
	LineItem *req;

	// time requirement
	req = new LineItem();
	req->Entity = Entities["Time"];
	if (req->Entity == NULL) {
	    list<string> typeFields;
	    typeFields.push_back("Time");

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = "Time";
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    Entities["Time"] = req->Entity;
	}
	req->Quantity = atoi(fields[14].c_str());
	entity->Requirements[0].push_back(req);

	// Skill requirement
	string skillName = fields[2];
	int skillLevel = atoi(fields[3].c_str());

	req = new LineItem();
	req->Entity = Entities[skillName];
	if (req->Entity != NULL) {
	    cout << "_";
	} else {
	    cout << "+";
	    list<string> typeFields;
	    typeFields.push_back("Skill");
	    typeFields.push_back(subtype);
	    typeFields.push_back(skillName);

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = skillName;
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    req->Entity->ProcessedSpreadsheetDefinition = false;
	    Entities[skillName] = req->Entity;
	}
	req->Quantity = skillLevel;
	entity->Requirements[0].push_back(req);
	
	// now all the components - we only have three per recipe right now but I think there
	// is space in here for four.
	for (int componentOffset = 5; componentOffset < 10; componentOffset += 2) {
	    if (fields[componentOffset].size() < 1) { continue; }
	    string componentName = fields[componentOffset];

	    req = new LineItem();
	    req->Entity = Entities[componentName];

	    if (componentName == "Yew Stave") {
		cout << endl << "Yew Stave Component" << endl;
	    }

	    if (req->Entity != NULL) {
		cout << "@";
	    } else {
		cout << "P";
		list<string> typeFields;
		typeFields.push_back("Item");
		typeFields.push_back(componentName);

		req->Entity = new EntityDefinition();
		req->Entity->Name = componentName;
		req->Entity->Type = typeHelper->GetType(typeFields);
		req->Entity->ProcessedSpreadsheetDefinition = false;
		Entities[componentName] = req->Entity;
	    }
	    req->Quantity = atoi(fields[componentOffset+1].c_str());
	    entity->Requirements[0].push_back(req);	    
	}
	
	delete name;
	// cout << "[" << *name << "] <-> [" << fields[15] << "]" << endl;
    }
    

    fin.close();

    return true;
}

