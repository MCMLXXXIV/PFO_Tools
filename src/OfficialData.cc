#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "OfficialData.h"
#include "EntityDefinition.h"
#include "RequirementNode.h"
#include "ProvidesNode.h"

using namespace std;

OfficialData::OfficialData() {
    FileProcessorMap["Recipes (Crafting).csv"] = &OfficialData::ParseAndStoreCraftingRecipeFile;
    FileProcessorMap["Recipes (Refining).csv"] = &OfficialData::ParseAndStoreRefiningRecipeFile;
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
    return this->ParseAndStoreRecipeFile(fn, Craft);
}

bool OfficialData::ParseAndStoreRefiningRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, Refine);
}

bool OfficialData::ParseAndStoreRecipeFile(string fn, EntitySubType subtype) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    cout << "+++ RAN: OfficialData::ParseAndStoreRecipeFile()" << endl;

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

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	map< string, EntityDefinition* >::iterator entityMapEntry;
	EntityDefinition *entity;
	entityMapEntry = Entities.find(fields[1]);
	if (entityMapEntry != Entities.end()) {
	    entity = (*entityMapEntry).second;
	    cout << ".";
	} else {
	    cout << "+";
	    // I could (should?) check that these fields were set correctly the first time - but lets call that a todo
	    entity = new EntityDefinition();
	    entity->Name = fields[1];
	    entity->Type = Recipe;
	    entity->SubType = subtype;

	    // TODO FIXME TEST
	    // soon I'll add code to consume the secondary, crowd sourced, table that give the yield
	    // for refining stuff - for now, in order to do some testing of the goal solution code,
	    // I'm going to make it a random number
	    if (subtype == Refine) {
		entity->CreationIncrement = (rand() % 4) + 1;
	    } else {
		entity->CreationIncrement = 1;
	    }
	    entity->Universal = false;

	    Entities[fields[1]] = entity;
	}
	entity->ProcessedSpreadsheetDefinition = true;
	
	RequirementNode *req;

	// time requirement
	req = new RequirementNode();
	req->Type = Time;
	req->Entity = NULL;
	req->QuantityValue._lval = atol(fields[14].c_str());
	entity->Requirements.push_back(req);

	// Skill requirement
	string skillName = fields[2] + " " + fields[3];
	req = new RequirementNode();
	req->Type = Entity;
	EntityDefinition *reqEntity;
	entityMapEntry = Entities.find(skillName);
	if (entityMapEntry != Entities.end()) {
	    reqEntity = (*entityMapEntry).second;
	    cout << ".";
	} else {
	    cout << "+";
	    reqEntity = new EntityDefinition();
	    reqEntity->Name = skillName;
	    reqEntity->Type = Skill;
	    reqEntity->SubType = Craft;
	    reqEntity->ProcessedSpreadsheetDefinition = false;
	    reqEntity->Universal = true;
	    Entities[skillName] = reqEntity;
	}
	req->Entity = reqEntity;
	entity->Requirements.push_back(req);
	
	// now all the components - we only have three per recipe right now but I think there
	// is space in here for four.
	for (int componentOffset = 5; componentOffset < 10; componentOffset += 2) {
	    if (fields[componentOffset].size() < 1) { continue; }
	    string componentName = fields[componentOffset];
	    entityMapEntry = Entities.find(componentName);
	    if (entityMapEntry != Entities.end()) {
		reqEntity = (*entityMapEntry).second;
		cout << ".";
	    } else {
		cout << "+";
		reqEntity = new EntityDefinition();
		reqEntity->Name = componentName;
		reqEntity->Type = Item;
		reqEntity->SubType = None;
		reqEntity->ProcessedSpreadsheetDefinition = false;
		reqEntity->Universal = false;
		Entities[componentName] = reqEntity;
	    }
	    req = new RequirementNode();
	    req->Type = Entity;
	    req->Entity = reqEntity;
	    req->QuantityValue._lval = atol(fields[componentOffset+1].c_str());
	    entity->Requirements.push_back(req);	    
	}
	// cout << "[" << fields[1] << "] <-> [" << fields[15] << "]" << endl;
    }
    

    fin.close();

    return true;
}

