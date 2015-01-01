#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <map>
#include <string>
#include <strings.h>

#include "OfficialData.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"
#include "LineItem.h"
#include "Utils.h"
#include "Log.h"

// ---------------------------------------------------------------------------------------------- //
//Stephen Cheney Jun 25, 2014, 09:53 AM
//http://paizo.com/threads/rzs2rney?All-0-refining-recipes-granted-free-by-trainers#2
//
//Roughly a quarter of all recipes are set to "default"; you get them as soon as you get the
//required rank. For refining, the +0 is always default, and the +1 to +3 are always learned. For
//crafting, it's significantly more arbitrary, but should still make up roughly a quarter of the
//available recipes. Skills with more recipes will get more default recipes. All crafting and
//refining skills should have at least one default recipe that comes in at rank 0 so you can try
//out the discipline before you buy. They should all be more-or-less evenly distributed across the
//whole progression, but since the default recipes tend to use a lower rank requirement, they may
//cluster toward the beginning of each tier. 
//
//http://paizo.com/threads/rzs2rney?All-0-refining-recipes-granted-free-by-trainers#4
//For crafting, default = common. For refining, it's an arbitrary relationship between default and
//common. This is to make sure there are enough commons at lower level (since if all default
//refining was common, you'd never be able to get a common achievement past +0).
// ---------------------------------------------------------------------------------------------- //

// grep Artificer_1_C official_data/*
// grep Artificer_1_c official_data/*
// there is at least one case where an achievement name seems to be case insensitive:
// Artificer_1_C vs Artificer_1_c
//             ^                ^
// Also, "personality" and "Personality" exist in the data.
//
// Therefore I've just added a case-insensitive comparator to the keeper/organizer of the
// entities.  All calls to find an Entity by name use my map<string,Entity> so now, with the
// case-insensitive comparator, personality and Personality will not be differentiated.

using namespace std;
using namespace boost;

OfficialData* OfficialData::m_pInstance = NULL;
OfficialData* OfficialData::Instance() {
    if (!m_pInstance) {
	m_pInstance = new OfficialData();
    }
    return m_pInstance;
}

OfficialData::OfficialData() {
    FileProcessorMap["Recipes (Crafting).csv"] = &OfficialData::ParseAndStoreCraftingRecipeFile;
    FileProcessorMap["Recipes (Refining).csv"] = &OfficialData::ParseAndStoreRefiningRecipeFile;
    FileProcessorMap["Skills Advancement.csv"] = &OfficialData::ParseAndStoreSkillsAdvancementFile;
    FileProcessorMap["Armor Advancement.csv"] = &OfficialData::ParseAndStoreArmorAdvancementFile;
    FileProcessorMap["Attack Advancement.csv"] = &OfficialData::ParseAndStoreAttackAdvancementFile;
    FileProcessorMap["Bonuses Advancement.csv"] = &OfficialData::ParseAndStoreBonusesAdvancementFile;
    FileProcessorMap["Cantrip Advancement.csv"] = &OfficialData::ParseAndStoreCantripAdvancementFile;
    FileProcessorMap["Defensive Advancement.csv"] = &OfficialData::ParseAndStoreDefensiveAdvancementFile;
    FileProcessorMap["Expendables Advancement.csv"] = &OfficialData::ParseAndStoreExpendablesAdvancementFile;
    FileProcessorMap["Feature Advancement.csv"] = &OfficialData::ParseAndStoreFeatureAdvancementFile;
    FileProcessorMap["Orison Advancement.csv"] = &OfficialData::ParseAndStoreOrisonAdvancementFile;
    FileProcessorMap["Points Advancement.csv"] = &OfficialData::ParseAndStorePointsAdvancementFile;
    FileProcessorMap["Proficiencies.csv"] = &OfficialData::ParseAndStoreProficienciesAdvancementFile;
    FileProcessorMap["Reactive Advancement.csv"] = &OfficialData::ParseAndStoreReactiveAdvancementFile;
    FileProcessorMap["Utility Advancement.csv"] = &OfficialData::ParseAndStoreUtilityAdvancementFile;
    FileProcessorMap["Feat Achievements.csv"] = &OfficialData::ParseAndStoreFeatAchievements;
    FileProcessorMap["RecipeYields_Crowdforged.csv"] = &OfficialData::ParseAndStoreCrowdforgedRecipeDataFile;
}

bool OfficialData::ParseAndStoreSkillsAdvancementFile(string fn, bool t)        { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreArmorAdvancementFile(string fn, bool t)         { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreAttackAdvancementFile(string fn, bool t)        { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreBonusesAdvancementFile(string fn, bool t)       { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreCantripAdvancementFile(string fn, bool t)       { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreDefensiveAdvancementFile(string fn, bool t)     { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreExpendablesAdvancementFile(string fn, bool t)   { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreOrisonAdvancementFile(string fn, bool t)        { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStorePointsAdvancementFile(string fn, bool t)        { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreReactiveAdvancementFile(string fn, bool t)      { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreUtilityAdvancementFile(string fn, bool t)       { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreFeatureAdvancementFile(string fn, bool t)       { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}
bool OfficialData::ParseAndStoreProficienciesAdvancementFile(string fn, bool t) { return this->ParseAndStoreProgressionFile(fn, t, "Feat");}

int OfficialData::ProcessSpreadsheetDir(string directoryName) {
    DIR *dp;
    struct dirent *dirp;

    dp = opendir(directoryName.c_str());
    if (dp == NULL) {
	Logger::Instance()->Log(Logger::Level::Error, "", "Failed to open %s: %s (%d)\n", directoryName.c_str(), strerror(errno), errno);
	return errno;
    }

    map<string,string> delayed;
    while ((dirp = readdir(dp))) {
	struct stat filestat;

	string fname = dirp->d_name;
	string filepath = directoryName + "/" + fname;

	if (stat( filepath.c_str(), &filestat)) continue;
	if (S_ISDIR( filestat.st_mode)) continue;

	// DELAY: I'm delaying this because I want to verify that this crowd forged data dump
	// at least has the right item names - and I can't do that until I've parsed all
	// the items.
	if (fname == "RecipeYields_Crowdforged.csv") {
	    delayed[fname] = filepath;
	    continue;
	}

	if (FileProcessorMap[fname]) {
	    (this->*FileProcessorMap[fname])(filepath, false);
	} else {
	    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "no handler for %s\n", fname.c_str());
	}	
    }
    closedir(dp);

    // DELAY note in loop above
    map<string,string>::iterator itr;
    for (itr = delayed.begin(); itr != delayed.end(); ++itr) {
	if (FileProcessorMap[(*itr).first]) {
	    (this->*FileProcessorMap[(*itr).first])((*itr).second, false);
	} else {
	    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "no handler for %s\n", (*itr).first.c_str());
	}	
    }	

    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "Done processing spreadsheet data; have %d entities\n", Entities.size());
    return 0;
}

bool OfficialData::TestParseFile(string filepath) {
    // open the file named fn
    // parse dump the data cells to stdout

    Logger *log = Logger::Instance();

    filesystem::path path(filepath);
    string fname = path.leaf().string();

    if (FileProcessorMap[fname]) {
	(this->*FileProcessorMap[fname])(filepath, true);
    } else {
	log->Log(Logger::Level::Warn, "OfficialData", "no handler for %s\n", fname.c_str());
    }
    
    return true;
}

EntityDefinition* OfficialData::GetEntity(string fullyQualifiedName) {
    map< string, EntityDefinition* >::iterator entityMapEntry;
    entityMapEntry = Entities.find(fullyQualifiedName);
    if (entityMapEntry == Entities.end()) {
	return NULL;
    } else {
	return (*entityMapEntry).second;
    }
}

void OfficialData::SearchForItemsThatRequire(EntityDefinition* targetEntity) {
    bool foundOne = false;
    set<EntityDefinition*> searched;
    map< string, EntityDefinition* >::iterator tlItr;
    for (tlItr = Entities.begin(); tlItr != Entities.end(); ++tlItr) {
	string key = (*tlItr).first;
	EntityDefinition* entity = (*tlItr).second;
	if (entity->HasRequirement(targetEntity, searched)) {
	    // this one is just a cout because it's the requsted output - not logging
	    cout << key << " / " << entity->Name << " has this as a requirement" << endl;
	    foundOne = true;
	}
    }
    if (foundOne == false) {
	cout << "didn't find this as a requirement - searched " << searched.size() << " entities" << endl;
    }
}

vector<string> OfficialData::SearchForEntitiesMatchingStrings(const char *buf)
{
    //     map< string, EntityDefinition*, comp> Entities;

    vector<string> matchStringsCased = Utils::SplitDelimitedValues(buf, " ");
    if (matchStringsCased.size() == 0) {
	// I'm not really returning "matchStringsCased" but just an empty vector
	return matchStringsCased;
    }

    vector<string> matchStrings;
    vector<string>::iterator itr = matchStringsCased.begin();
    for(; itr != matchStringsCased.end(); ++itr) {
	matchStrings.push_back(to_lower_copy(*itr));
    }

    vector<string>::iterator requiredString = matchStrings.begin();
    list<string> matchingEntities;
    int searchCount = 0;
    for(; requiredString != matchStrings.end(); ++requiredString) {
	if ((*requiredString).length() == 0) { continue; }
	if (searchCount == 0) {
	    map<string, EntityDefinition*>::iterator itr = Entities.begin();
	    for(; itr != Entities.end(); ++itr) {
		string entityName = to_lower_copy((*itr).first);		
		if (entityName.find(*requiredString) != string::npos) {
		    matchingEntities.push_back((*itr).first);
		}
	    }
	} else {
	    if (matchingEntities.size() == 0) { break; }
	    // here we will go through matchingEntities and remove any entry that doesn't match *requiredString
	    // we can't remove items from a thing we are iterating over so we will make a list of items to
	    // remove and then remove them all in a separate loop after.
	    
	    vector<string> itemsForRemoval;
	    list<string>::iterator entityEntry = matchingEntities.begin();
	    for(; entityEntry != matchingEntities.end(); ++entityEntry) {
		string entityName = to_lower_copy(*entityEntry);
		if (entityName.find(*requiredString) == string::npos) {
		    itemsForRemoval.push_back(*entityEntry);
		}
	    }
	    if (itemsForRemoval.size() > 0) {
		vector<string>::iterator removeMe = itemsForRemoval.begin();
		for(; removeMe != itemsForRemoval.end(); ++removeMe) {
		    matchingEntities.remove(*removeMe);
		}
	    }
	}
	++searchCount;
    }

    vector<string> retVal;
    matchingEntities.sort();
    list<string>::iterator matchingEntry = matchingEntities.begin();
    for(; matchingEntry != matchingEntities.end(); ++matchingEntry) {
	retVal.push_back(*matchingEntry);
    }
    return retVal;
}

void OfficialData::Dump() {

    cout << "Have " << Entities.size() << " entities" << endl;

    map<string, EntityDefinition*>::iterator itr;
    for (itr = Entities.begin(); itr != Entities.end(); ++itr) {
	cout << *((*itr).second) << endl;
    }
}

bool OfficialData::ParseAndStoreCraftingRecipeFile(string fn, bool testRun) {
    return this->ParseAndStoreRecipeFile(fn, testRun, "Craft");
}

bool OfficialData::ParseAndStoreRefiningRecipeFile(string fn, bool testRun) {
    return this->ParseAndStoreRecipeFile(fn, testRun, "Refine");
}

// I used to do different stuff if the recipe file was crafting vs refining - but I've
// decided to try it a different way - but I'm leaving the arg in, even if it's ignored,
// for the time being.
bool OfficialData::ParseAndStoreRecipeFile(string fn, bool testRun, string ignored) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    // we are in the recipe file parser so we are only filling out entities and entity requirements
    // for items.  And all items have ranks (even though the Item Type for now returns false via
    // the entity type helper IsRanked() method).  Currently items have ranks +0, +1, +2, +3, +4,
    // and +5.  Only the refining recipes will have ranks in the name.  As a consequence, only the
    // refining items will have separate Requirement lists and Provides lists for each rank.  The
    // rank of a crafted item is determined solely based on the (average?) ranks of the refined
    // items used to craft it.  That is, if a Sunrod is created w/ all +3 materials (15 of Weak
    // Luminous Extract +3, 2 of Weak Acidic Extract +3, and one Copper Bar +3) then the resulting
    // Sunrod will be +3.

    // Also, +0 is Rank Zero (Rank = 0).

    // cout << "+++ RUNNING: OfficialData::ParseAndStoreRecipeFile(" << fn << ")" << endl;

    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();
    Logger *log = Logger::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    vector<string> colName;
    int colNameMaxLen = 0;
    while(getline(fin, line)) {
	// Recipe Name,Feat Name,Feat Rank,Tier,Component 1,#1,Component 2,#2,Component 3,#3,Component 4,#4,Output,,#,Base Crafting Seconds,Category,+0 Quality,Achievement Type,Last Updated,Template|R...

	//[ 0]           Recipe Name Weak Soothing Extract +3
	//[ 1]             Feat Name Apothecary
	//[ 2]             Feat Rank 5.0
	//[ 3]                  Tier 1.0
	//[ 4]               Stock 1 Weak Soothing
	//[ 5]                    #1 12.0
	//[ 6]               Stock 2 Ordered Essence
	//[ 7]                    #2 5.0
	//[ 8]               Stock 3
	//[ 9]                    #3
	//[10]               Stock 4
	//[11]                    #4
	//[12]                Output Weak Soothing Extract
	//[13]               Upgrade 3.0
	//[14]                     # 3.0
	//[15] Base Crafting Seconds 200.0
	//[16]               Quality 72.0
	//[17]               Variety Chemical
	//[18]      Achievement Type Uncommon
	//[19]          Last Updated 11-24-14
	//[20] Template|Recipe Name| Recipe|Weak Soothing Extract +3|Apothecary|5|1|Weak Soothing|12|Ordered Essence|5|||||Uncommon|200|11-24-14

	++line_num;

	// skip the first line - its a header line
	if (line_num < 2) {
	    colName = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());
	    vector<string>::iterator colNm = colName.begin();
	    for(; colNm != colName.end(); ++colNm) {
		int len = (*colNm).length();
		if (len < 100 && len > colNameMaxLen) {
		    colNameMaxLen = len;
		    log->Log(Logger::Level::Verbose, "OfficialData",
			     "longer column name: %s\n", (*colNm).c_str());
		}
	    }
	    log->Log(Logger::Level::Verbose, "OfficialData",
		     "Parsed column names from header - have %d column names\n", colName.size());

	    for(uint idx = 0; idx < colName.size(); ++idx) {
		log->Log(Logger::Level::Verbose, "OfficialData",
			 "column %2u: %s\n", idx, colName[idx].c_str());
	    }		
	    continue;
	}

	//stringstream linestream(line);
	string val;
	vector<string> fields = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());
	
	// old way, pre Dec 2014.  Then the fields were poorly delimited by a '|'
	//while (getline(linestream, val, '|')) {
	//   fields.push_back(val);
	//}

	if (testRun == true) {
	    char diagFormatStr[1024];
	    snprintf(diagFormatStr, 1023, "[%%2u] %%%d.%ds %%s\n", colNameMaxLen, colNameMaxLen);
	    printf("----------------------------------------------------------------------------------------------------------------\n");
	    for(uint idx = 0; idx < fields.size(); ++idx) {
		printf(diagFormatStr, idx, colName[idx].c_str(), fields[idx].c_str());
	    }
	    continue;
	}
	
	if (fields.size() != 21) {
	    if (fields.size() >= 18) {
		log->Log(Logger::Level::Warn, "OfficialData",
			 "%s:%d doesn't have 21 fields - has %d - missing the 'Last Updated' field - but we don't use that: %s\n",
			 fn.c_str(), line_num, fields.size(), line.c_str());
	    } else {
		log->Log(Logger::Level::Warn, "OfficialData",
			 "%s:%d doesn't have 21 fields - has %d : %s\n",
			 fn.c_str(), line_num, fields.size(), line.c_str());
		continue;
	    }
	}
	if (fields[0].size() < 1) {
	    log->Log(Logger::Level::Warn, "OfficialData", "%s:%d has %d columns but an empty name : %s\n", fn.c_str(), line_num, fields.size(), line.c_str());
	    continue;
	}

	string *name = new string(fields[0]);
	unsigned rank = 0;
	char *namePart;
	if (Utils::RankInName(name->c_str(), &namePart, rank)) {
	    delete name;
	    name = new string(namePart);
	    delete namePart;
	} else {
	    rank = 0;
	}

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	EntityDefinition *entity = NULL;
	string fullyQualifiedName = "Item.";
	fullyQualifiedName += *name;
	entity = GetEntity(fullyQualifiedName);
	if (entity == NULL) {
	    //cout << "+";
	    // I could (should?) check that these fields were set correctly the first time - but lets call that a todo
	    entity = new EntityDefinition();
	    entity->Name = *name;

	    list<string> typeFields;
	    typeFields.push_back("Item");
	    typeFields.push_back(*name);
	    entity->Type = typeHelper->GetType(typeFields);
	    StoreEntity(fullyQualifiedName, entity);
	} // else { cout << "."; }

	// we only add a Requirements and Provides list here where we are processing the Entity
	// from the spreadsheet
	while (entity->Requirements.size() <= rank) {
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));
	}

	entity->CreationIncrement = atoi(fields[14].c_str());

	entity->ProcessedSpreadsheetDefinition = true;
	
	LineItem *req;

	// time requirement
	req = new LineItem();
	req->Entity = GetEntity("Time");
	if (req->Entity == NULL) {
	    list<string> typeFields;
	    typeFields.push_back("Time");

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = "Time";
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    StoreEntity("Time", req->Entity);
	}
	req->Quantity = atoi(fields[15].c_str());
	entity->Requirements[rank].push_back(req);

	// Feat requirement
	string featName = fields[1];
	int featLevel = atoi(fields[2].c_str());

	string featNameFqn = "Feat." + featName;
	req = new LineItem();
	req->Entity = GetEntity(featNameFqn);
	if (req->Entity != NULL) {
	    //cout << "_";
	    list<string> typeFields;
	    typeFields.push_back("Feat");
	    typeFields.push_back(featName);
	    req->Entity->Type = typeHelper->GetType(typeFields);

	} else {
	    //cout << "+";
	    list<string> typeFields;
	    typeFields.push_back("Feat");
	    typeFields.push_back(featName);

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = featName;
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    req->Entity->ProcessedSpreadsheetDefinition = false;
	    StoreEntity(featNameFqn, req->Entity);
	}
	req->Rank = featLevel;

	entity->Requirements[rank].push_back(req);
	
	// now all the components - we only have three per recipe right now but I think there
	// is space in here for four.
	for (int componentOffset = 4; componentOffset < 12; componentOffset += 2) {
	    if (fields[componentOffset].size() < 1) { continue; }
	    string componentName = fields[componentOffset];
	    string componentNameFqn = "Item." + componentName;
	    req = new LineItem();

	    req->Entity = GetEntity(componentNameFqn);

	    if (req->Entity == NULL) {
		list<string> typeFields;
		typeFields.push_back("Item");
		typeFields.push_back(componentName);

		req->Entity = new EntityDefinition();
		req->Entity->Name = componentName;
		req->Entity->Type = typeHelper->GetType(typeFields);
		req->Entity->ProcessedSpreadsheetDefinition = false;
		req->Entity->CreationIncrement = 1;
		StoreEntity(componentNameFqn, req->Entity);
	    }
	    req->Quantity = atoi(fields[componentOffset+1].c_str());
	    entity->Requirements[rank].push_back(req);	    
	}
	
	delete name;
	// cout << "[" << *name << "] <-> [" << fields[15] << "]" << endl;
    }
    

    fin.close();

    return true;
}

bool OfficialData::ParseAndStoreProgressionFile(string fn, bool testRun, string t) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    // cout << "+++ RUNNING: OfficialData::ParseAndStoreSkillFile(" << fn << ")" << endl;

    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();
    Logger *log = Logger::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	log->Log(Logger::Level::Error, "", "Failed to read file %s: %s (%d)\n", fn.c_str(), strerror(errno), errno);
	return false;
    }
    string line;
    int line_num = 0;
    uint slotNameColumn = 0;
    while(getline(fin, line)) {
	++line_num;
	// there is only one line per feat with each line having the data out to the max feat (121 total fields)
	vector<string> fields = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());

	if (fields.size() < 1) {
	    log->Log(Logger::Level::Warn, "OfficialData", "%s:%d is empty.  Skipping...\n", fn.c_str(), line_num);
	    assert(line_num > 1);
	    continue;
	}
	
	if (fields[0].size() == 0) {
	    log->Log(Logger::Level::Warn, "OfficialData", "%s:%d has %d columns but an empty name.  Skipping...\n", fn.c_str(), line_num, fields.size());
	    assert(line_num > 1);
	    continue;
	}

	//Index (but only in Points Advancement.csv as of 26 Oct dump)
	//SlotName
	//Exp Lv1
	//Category Lv1
	//Feat Lv1
	//Achievement Lv1
	//AbilityReq Lv1
	//AbilityBonus Lv1
	//Exp Lv2
	//Category Lv2
	//Feat Lv2
	//Achievement Lv2
	//AbilityReq Lv2
	//AbilityBonus Lv2

	if (line_num == 1) {
	    // cout << "skipping first line w/ first field: [" << featName << "]" << endl;
	    // find the "SlotName" column and skip the line
	    // I could make this parser digest the header column names and then fetch the cells
	    // by refering to the name - but that would be just as brittle because there is no
	    // guarantee that Goblinworks won't make an unannounced change to that also
	    for (uint idx = 0; idx < fields.size(); ++idx) {
		if (fields[idx] == "SlotName") {
		    slotNameColumn = idx;
		    // just to be aware when things change again
		    assert(slotNameColumn < 2);
		    break;
		}
	    }
	    continue;
	}

	string featName = fields[slotNameColumn];

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	EntityDefinition *entity;
	string featNameFQ = "Feat.";
	featNameFQ += featName;
	entity = GetEntity(featNameFQ);
	if (entity != NULL) {
	    // printf("%c %2d: %22s -> %d\n", '.', line_num, fields[0].c_str(), (int)fields.size());
	    if (fn == "official_data/Utility Advancement.csv" && featName == "Channel Smite") {
		log->Log(Logger::Level::Note, "knownParseErrors", "%s:%d Utility Advancement.csv has two rows for Channel Smite...\n", fn.c_str(), line_num);
	    
	    } else {
		assert(entity->ProcessedSpreadsheetDefinition == false);
	    }
	} else {
	    //printf("%c %2d: %22s -> %d\n", '+', line_num, fields[0].c_str(), (int)fields.size());

	    entity = new EntityDefinition();
	    entity->Name = featName;

	    list<string> typeFields;
	    typeFields.push_back(t);
	    typeFields.push_back(featName);
	    entity->Type = typeHelper->GetType(typeFields);
	    StoreEntity(featNameFQ, entity);
	}
	entity->ProcessedSpreadsheetDefinition = true;
	    
	// hack here to keep track of known errors in the data
	if (entity->Requirements.size() != 0) {
	    if (fn == "official_data/Utility Advancement.csv" && featName == "Channel Smite") {
		log->Log(Logger::Level::Note, "knownParseErrors", "%s:%d Utility Advancement.csv has two rows for Channel Smite...\n", fn.c_str(), line_num);
	    } else {
		// feats are ranked - always ranked - but we won't add the ranks of requirements or
		// provides until we process them here
		assert (entity->Requirements.size() == 0);
	    }
	}


	// notes on gotAtLeastOneXpValue and gotOneNoXpSet
	// The latest official data from goblin works had a lot of trailing columns of empty data.
	// I didn't want to add a bunch of bogus entries to the requirements lists and provides
	// lists so I short circuited if the set had an empty field for XP.  And just to be sure
	// I wasn't skipping something real, I also added an assertion that should assert if I
	// find any good rows after a bad row - after all, maybe one day a zero length xp field
	// may not be a good indicator that everything is blank.

	uint idx;
	int rank;
	bool gotAtLeastOneXpValue = false;
	bool gotOneNoXpSet = false;
	for (idx = slotNameColumn + 1, rank = 1; idx < fields.size(); idx += 6, ++rank) {

	    // [  0 ][  1  ][  2  ][  3  ][  4  ][  5  ][  6  ][  7  ][  8  ][  9  ][ 10  ][ 11  ][ 12  ][ 13  ]
	    // [name][1_exp][1_cat][1_fea][1_ach][1_abi][1_ab+][2_exp][2_cat][2_fea][2_ach][2_abi][2_ab+]

	    // [  0 ][  1  ][  2  ][  3  ][  4  ][  5  ][  6  ][  7  ][ 163 ][ 164 ][ 165 ][ 166 ][ 167 ][ 168 ]
	    // [indx][name ][1_exp][1_cat][1_fea][1_ach][1_abi][1_ab+][2_exp][2_cat][2_fea][2_ach][2_abi][2_ab+]
	    // say: idx = 7 and size = 11 (error).  11(sz) - 7(ix) = 4
	    // say: idx = 163 and size = 168 - error as there is no element 168 which would be the fith field
	    if ((fields.size() - idx) < 6) {
		log->Log(Logger::Level::Warn, "OfficialData",
					"%s:%d  %s rank %d has incomplete data - skipping...\n",
					fn.c_str(), line_num, featName.c_str(), rank);
		continue;
	    }

	    if (fields[idx].size() > 0) {
		assert(gotOneNoXpSet == false);
		gotAtLeastOneXpValue = true;
	    }
	    if (gotAtLeastOneXpValue == true && fields[idx].size() < 1) {
		log->Log(Logger::Level::Warn, "OfficialData",
					"%s:%d  %s rank %d has incomplete data (no xp) - skipping...\n",
					fn.c_str(), line_num, featName.c_str(), rank);
		gotOneNoXpSet = true;
		continue;
	    }

	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));

	    if (rank == 1) {
		// we will index into the requirements based on the rank since the first rank
		// is 1, we need to add an empty/dummy requirement list at rank 0 so that
		// Requirements[1] gets the first rank's requirements.
		entity->Requirements.push_back(*(new list<LineItem*>));
		entity->Provides.push_back(*(new list<LineItem*>));
	    }
	    list<LineItem*> *reqs = &(entity->Requirements.back());
	    list<LineItem*> *pros = &(entity->Provides.back());
	    
	    // add the exp requirement
	    string entityName = "ExperiencePoint";
	    LineItem *req = new LineItem();
	    req->Entity = GetEntity(entityName);
	    if (req->Entity == NULL) {
		list<string> typeFields;
		typeFields.push_back(entityName);
		
		req->Entity = new EntityDefinition();
		req->Entity->Name = entityName;
		req->Entity->Type = typeHelper->GetType(typeFields);
		StoreEntity(entityName, req->Entity);
	    }
	    req->Quantity = atoi(fields[idx].c_str());
	    reqs->push_back(req);

	    // add the previous rank as a requirement
	    if (rank > 1) {
		req = new LineItem(entity, (rank - 1), 1);
		reqs->push_back(req);
	    }

	    // add the achievement point
	    string reqStr = fields[idx+1];
	    string label = "AchievementPoint";
	    string errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    log->Log(Logger::Level::Warn, "OfficialData",
					    "%s:%d  failed to parse this %s requirement string: [%s]; err: %s - skipping...\n",
					    fn.c_str(), line_num, label.c_str(), reqStr.c_str(), errMsg.c_str());
		} else {
		    reqs->push_back(required);
		}
	    }

	    // add the feat requirements
	    reqStr = fields[idx+2];
	    label = "Feat";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    log->Log(Logger::Level::Warn, "OfficialData",
					    "%s:%d  failed to parse this %s requirement string: [%s]; err: %s - skipping...\n",
					    fn.c_str(), line_num, label.c_str(), reqStr.c_str(), errMsg.c_str());
		} else {
		    reqs->push_back(required);
		}
	    }

	    
	    // add the achievement requirements
	    // Fighter is listed as both Achievement and Feat
	    reqStr = fields[idx+3];
	    label = "Achievement"; // eg, Shield Expert 4 - this is an achievement
	    //label = "Feat"; // not sure why I had this as a Feat here
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    log->Log(Logger::Level::Warn, "OfficialData",
					    "%s:%d  failed to parse this %s requirement string: [%s]; err: %s - skipping...\n",
					    fn.c_str(), line_num, label.c_str(), reqStr.c_str(), errMsg.c_str());
		} else {
		    reqs->push_back(required);
		}
	    }

	    // add the ability score requirements
	    reqStr = fields[idx+4];
	    label = "AbilityScore";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    log->Log(Logger::Level::Warn, "OfficialData",
					    "%s:%d  failed to parse this %s requirement string: [%s]; err: %s - skipping...\n",
					    fn.c_str(), line_num, label.c_str(), reqStr.c_str(), errMsg.c_str());
		} else {
		    reqs->push_back(required);
		}
	    }

	    // add the ability bonus "Provide" node
	    reqStr = fields[idx+5];
	    label = "AbilityScore";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *provides = ParseRequirementString(reqStr, label, errMsg);
		if (provides == NULL) {
		    log->Log(Logger::Level::Warn, "OfficialData",
					    "%s:%d  failed to parse this %s bonus string: [%s]; err: %s - skipping...\n",
					    fn.c_str(), line_num, label.c_str(), reqStr.c_str(), errMsg.c_str());
		} else {
		    pros->push_back(provides);
		}
	    }
	    
	}
	
	// for (int idx = 0; idx < fields.size(); ++idx) { cout << fields[idx] << endl; }
    }

    fin.close();

    return true;
}

bool OfficialData::ParseAndStoreFeatAchievements(string fn, bool testRun) {
    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();
    Logger *log = Logger::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    while(getline(fin, line)) {
	++line_num;
	if (line_num == 1) {
	    assert(string("Display Name,Description,Feat Req List") == line);
	    continue;
	}

	vector<string> fields = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());
	assert(fields.size() == 3);

	unsigned rank;
	char *namePart;
	bool rankInName = Utils::RankInName(fields[0].c_str(), &namePart, rank);
	assert(rankInName == true);
	
	string featNameShort = namePart;
	delete namePart;

	string featNameFQ = "Achievement.";
	featNameFQ += featNameShort;

	EntityDefinition *entity = GetEntity(featNameFQ);
	if (entity == NULL) {
	    entity = new EntityDefinition();
	    entity->Name = featNameShort;

	    list<string> typeFields;
	    typeFields.push_back("Achievement");
	    typeFields.push_back(featNameShort);
	    entity->Type = typeHelper->GetType(typeFields);
	    StoreEntity(featNameFQ, entity);
	}
	entity->ProcessedSpreadsheetDefinition = true;

	while (entity->Requirements.size() <= rank) {
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));
	}

	list<LineItem*> *reqs = &(entity->Requirements[rank]);
	// we should only ever process the reqs for a given rank of a given entity once
	assert(reqs->size() == 0);

	// add the previous rank as a requirement
	if (rank > 1) {
	    LineItem *req = new LineItem(entity, (rank - 1), 1);
	    reqs->push_back(req);
	}
	
	string reqStr = fields[2];
	string label = "Feat";
	string errMsg = "";
	if (reqStr.size() > 0) {
	    LineItem *required = ParseRequirementString(reqStr, label, errMsg);
	    if (required == NULL) {
		log->Log(Logger::Level::Warn, "OfficialData", "failed to parse this %s requirement string: [%s]; err: %s\n", label.c_str(), reqStr.c_str(), errMsg.c_str());
		// cout << "ERROR: failed to parse this " << label << " requirement string: [" << reqStr << "]; err:" << errMsg << endl;
	    } else {
		reqs->push_back(required);
	    }
	}
    }
    fin.close();

    log->Log(Logger::Level::Note, "OfficialData", "parsed %s\n", fn.c_str());
    return true;
}

LineItem* OfficialData::ParseRequirementString(string reqStr, string entityTypeName, string &errMsg) {
    // TODO: MEMORY LEAKS HERE
    // if this function encounters an error (and returns NULL) midway through parsing
    // a requirement, we will leak the already-created LineItems and Logic entities.

    vector<string> andedGroups = Utils::SplitCommaSeparatedValuesWithQuotedFields(reqStr.c_str());
    if (andedGroups.size() < 1) {
	errMsg = "topLevelZero";
	return NULL;
    }

    vector<string>::iterator groupEntry;
    list<LineItem*> andedLineItems;
    for (groupEntry = andedGroups.begin(); groupEntry != andedGroups.end(); ++groupEntry) {
	if (strstr((*groupEntry).c_str(), " or ")) {
	    list<string> orEntityLongName = { string("LogicOr") };
	    EntityDefinition *orEntity = new EntityDefinition();
	    orEntity->Name = "LogicOr";
	    orEntity->Type = EntityTypeHelper::Instance()->GetType(orEntityLongName);
	    orEntity->Requirements.push_back(*(new list<LineItem*>));
	    
	    vector<string> orReqs = Utils::SplitDelimitedValues((*groupEntry).c_str(), " or ");
	    vector<string>::iterator orEntry;
	    for (orEntry = orReqs.begin(); orEntry != orReqs.end(); ++orEntry) {
		LineItem *newReq = BuildLineItemFromKeyEqualsVal((*orEntry), entityTypeName);
		if (newReq == NULL) {
		    errMsg = "failed to parse 'or' requirement";
		    return NULL;
		}
		orEntity->Requirements[0].push_back(newReq);
	    }
	    andedLineItems.push_back(new LineItem(orEntity, 0, 1));
	    
	} else {
	    LineItem *newReq = BuildLineItemFromKeyEqualsVal((*groupEntry), entityTypeName);
	    if (newReq == NULL) {
		// ignore empty entries - the given data has some trailing commas
		// EG: Feat Achievements.csv
		//    Willpower Bonus=7, Arcane Attack Bonus=7, Power=26,
		continue;
		//errMsg = "failed to parse 'and' requirement [";
		//errMsg += (*groupEntry);
		//errMsg += "]";
		//return NULL;
	    }
	    andedLineItems.push_back(newReq);
	}
    }
    if (andedLineItems.size() < 0) {
	errMsg = "failed to parse anything";
	return NULL;
    }

    if (andedLineItems.size() == 1) {
	return andedLineItems.front();
    } else {
	list<string> andEntityLongName = { string("LogicAnd") };
	EntityDefinition *andEntity = new EntityDefinition();
	andEntity->Name = "LogicAnd";
	andEntity->Type = EntityTypeHelper::Instance()->GetType(andEntityLongName);
	andEntity->Requirements.push_back(*(new list<LineItem*>));

	list<LineItem*>::iterator groupEntry;
	for (groupEntry = andedLineItems.begin(); groupEntry != andedLineItems.end(); ++groupEntry) {
	    andEntity->Requirements[0].push_back(*groupEntry);
	}
	LineItem *group = new LineItem(andEntity, 0, 1);
	return group;
    }
}

LineItem* OfficialData::BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName) {
    string key, value;

    if (!Utils::SplitKeyValueOnChar(kvp.c_str(), "=", key, value)) {
	return NULL;
    }

    // the input will be of the form "<EntityName>=<rank|Quantity>"
    // EG: Fighter=3  or  Strength=12

    string fqn = entityTypeName;
    fqn += "." + key;
    EntityDefinition* entity = GetEntity(fqn);
    if (entity == NULL) {
	entity = new EntityDefinition();
	entity->Name = key;
	list<string> typeFields;
	typeFields.push_back(entityTypeName);
	typeFields.push_back(key);
	entity->Type = EntityTypeHelper::Instance()->GetType(typeFields);
	StoreEntity(fqn, entity);
    }
    if (EntityTypeHelper::Instance()->IsRanked(entity->Type[0])) {
	return new LineItem(entity, atoi(value.c_str()), 1);
    } else {
	return new LineItem(entity, 0, atof(value.c_str()));
    }
}

bool OfficialData::StoreEntity(string fullyQualifiedName, EntityDefinition *entity) {
    map< string, EntityDefinition* >::iterator entityMapEntry;
    entityMapEntry = Entities.find(fullyQualifiedName);
    if (entityMapEntry == Entities.end()) {
	Entities[fullyQualifiedName] = entity;
	return true;
    } else {
	bool alreadyExists = true;
	assert(alreadyExists == false);
	return false;
    }
    return false;
}

bool OfficialData::ParseAndStoreCrowdforgedRecipeDataFile(string fn, bool testRun) {
    Logger *log = Logger::Instance();
    
    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    while(getline(fin, line)) {
	++line_num;
	if (line_num == 1) {
	    assert(string("Skill,Rank,Recipe Name,Qty Produced") == line);
	    continue;
	}

	vector<string> fields = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());
	assert(fields.size() == 4);
	
	string itemName = fields[2];
	EntityDefinition *entity = GetEntity("Item." + itemName);
	if (entity == NULL) {
	    bool foundEntity = (entity != NULL);
	    log->Log(Logger::Level::Error, "", "Can't find %s\n", itemName.c_str());
	    assert(foundEntity == true);
	}

	if (fields[3].size() > 0) {
	    int thirdPartyValue = atoi(fields[3].c_str());
	    if (thirdPartyValue != entity->CreationIncrement) {
		log->Log(Logger::Level::Note, "OfficialData", "%s has official Qty %d; UserRecorded Qty %d\n", itemName.c_str(), entity->CreationIncrement, thirdPartyValue);
	    }
	}
    }
    fin.close();
    log->Log(Logger::Level::Note, "OfficialData", "parsed %s\n", fn.c_str()); 
    return true;
}


