#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <boost/algorithm/string.hpp>

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

bool OfficialData::ParseAndStoreSkillsAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreArmorAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreAttackAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreBonusesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreCantripAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreDefensiveAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreExpendablesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
//bool OfficialData::ParseAndStoreFeatureAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreOrisonAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStorePointsAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
//bool OfficialData::ParseAndStoreProficienciesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreReactiveAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreUtilityAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}

bool OfficialData::ParseAndStoreFeatureAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreProficienciesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}


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

bool OfficialData::ParseAndStoreCrowdforgedRecipeDataFile(string fn) {
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
	    entity->CreationIncrement = atoi(fields[3].c_str());
	}
    }
    fin.close();
    log->Log(Logger::Level::Note, "OfficialData", "parsed %s\n", fn.c_str()); 
    return true;
}

bool OfficialData::ParseAndStoreFeatAchievements(string fn) {
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

	int rank;
	char *namePart;
	bool rankInName = Utils::RankInName(fields[0].c_str(), &namePart, rank);
	assert(rankInName == true);
	
	string featNameShort = namePart;
	delete namePart;

	string featNameFQ = "Feat.";
	featNameFQ += featNameShort;

	EntityDefinition *entity = GetEntity(featNameFQ);
	if (entity == NULL) {
	    entity = new EntityDefinition();
	    entity->Name = featNameShort;

	    list<string> typeFields;
	    typeFields.push_back("Feat");
	    typeFields.push_back(featNameShort);
	    entity->Type = typeHelper->GetType(typeFields);
	    StoreEntity(featNameFQ, entity);
	}
	entity->ProcessedSpreadsheetDefinition = true;

	if (entity->Requirements.size() == 0) {
	    // skipping rank zero
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));
	}
	while (entity->Requirements.size() <= (unsigned)rank) {
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));
	}

	list<LineItem*> *reqs = &(entity->Requirements[rank]);
	// we should only ever process the reqs for a given rank of a given entity once
	assert(reqs->size() == 0);

	// add the previous rank as a requirement
	if (rank > 1) {
	    LineItem *req = new LineItem(entity, (rank - 1));
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



void OfficialData::Dump() {

    cout << "Have " << Entities.size() << " entities" << endl;

    map<string, EntityDefinition*>::iterator itr;
    for (itr = Entities.begin(); itr != Entities.end(); ++itr) {
	cout << *((*itr).second) << endl;
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


EntityDefinition* OfficialData::GetEntity(string fullyQualifiedName) {
    map< string, EntityDefinition* >::iterator entityMapEntry;
    entityMapEntry = Entities.find(fullyQualifiedName);
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
	    (this->*FileProcessorMap[fname])(filepath);
	} else {
	    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "no handler for %s\n", fname.c_str());
	}	
    }
    closedir(dp);

    // DELAY note in loop above
    map<string,string>::iterator itr;
    for (itr = delayed.begin(); itr != delayed.end(); ++itr) {
	if (FileProcessorMap[(*itr).first]) {
	    (this->*FileProcessorMap[(*itr).first])((*itr).second);
	} else {
	    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "no handler for %s\n", (*itr).first.c_str());
	}	
    }	

    Logger::Instance()->Log(Logger::Level::Warn, "OfficialData", "Done processing spreadsheet data; have %d entities\n", Entities.size());
    return 0;
}

bool OfficialData::ParseAndStoreProgressionFile(string fn, string t) {
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
		req = new LineItem(entity, (rank - 1));
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

bool OfficialData::ParseProgressionFile(string fn) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    // cout << "+++ RUNNING: OfficialData::ParseAndStoreSkillFile(" << fn << ")" << endl;

    Logger *log = Logger::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
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

	//Index (but only in Points Advancement.csv)
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
	    // guarantee that Goblinworks won't fat finger that also
	    for (uint idx = 0; idx < fields.size(); ++idx) {
		if (fields[idx] == "SlotName") {
		    slotNameColumn = idx;
		    // just to be aware when things change again
		    assert(slotNameColumn < 2);
		    break;
		}
	    }
	    log->Log(Logger::Level::Verbose, "OfficialData", "slotNameColumn=%u\n", slotNameColumn);
	}

	string featName = fields[slotNameColumn];
	log->Log(Logger::Level::Verbose, "OfficialData", "FeatName:[%s]\n", featName.c_str());

	uint idx;
	int rank;
	for (idx = slotNameColumn + 1, rank = 1; idx < fields.size(); idx += 6, ++rank) {

	    // [  0 ][  1  ][  2  ][  3  ][  4  ][  5  ][  6  ][  7  ][  8  ][  9  ][ 10  ][ 11  ][ 12  ]
	    // [name][1_exp][1_cat][1_fea][1_ach][1_abi][1_ab+][2_exp][2_cat][2_fea][2_ach][2_abi][2_ab+]
	    // say: idx = 7 and size = 11 (error).  11(sz) - 7(ix) = 4
	    if ((fields.size() - idx) < 6) {
		log->Log(Logger::Level::Warn, "OfficialData",
					"%s:%d  %s rank %d has incomplete data - skipping...\n",
					fn.c_str(), line_num, featName.c_str(), rank);
	    }

	    vector<string> colNames = {"Exp", "Cat", "Fea", "ach", "aRq", "aBo"};
	    for (uint setIdx = 0; setIdx < 6 && idx+setIdx < fields.size(); ++setIdx) {
		cout << colNames[setIdx] << " " << rank << " [" << fields[idx+setIdx] << "]" << endl;
	    }
	}
    }
    fin.close();
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
	    andedLineItems.push_back(new LineItem(orEntity, 1));
	    
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
	LineItem *group = new LineItem(andEntity, 1);
	return group;
    }
}


LineItem* OfficialData::BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName) {
    string key, value;

    if (!Utils::SplitKeyValueOnChar(kvp.c_str(), "=", key, value)) {
	return NULL;
    }

    // Yikes - this one is scary - I can't remember if the type will be underspecified here
    // eg, will we store Item.Craft.Sword and then just get Item.Sword through this interface?
    // TODO: Dump all entities and make sure they are all correct...
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
    
    return new LineItem(entity, atof(value.c_str()));
}


bool OfficialData::ParseAndStoreCraftingRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Craft");
}

bool OfficialData::ParseAndStoreRefiningRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Refine");
}

// going to handle subtype differently - later I'll add a list of tags in the EntityDefinition
// then I can add the "Craft" tag or the "Refine" tag to the Entity.  But for now, this second arg
// is (almost) ignored.  Until I am parsing the file that tells me how much an item is created
// when you do one creation job (eg, if you make course thread, you get about 20 of them per job)
// I am adding dummy, random values when this function is called with ignored == "Refine".
bool OfficialData::ParseAndStoreRecipeFile(string fn, string ignored) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

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
	    log->Log(Logger::Level::Warn, "OfficialData",
		     "%s:%d doesn't have 16 fields - has %d : %s\n",
		     fn.c_str(), line_num, fields.size(), line.c_str());
	    continue;
	}
	if (fields[1].size() < 1) {
	    log->Log(Logger::Level::Warn, "OfficialData", "%s:%d has %d columns but an empty name : %s\n", fn.c_str(), line_num, fields.size(), line.c_str());
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

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	map< string, EntityDefinition* >::iterator entityMapEntry;
	EntityDefinition *entity;
	string fullyQualifiedName = "Item.";
	fullyQualifiedName += *name;
	entity = GetEntity(fullyQualifiedName);
	if (entity != NULL) {
	    // only add a Requirements and Provides list if we process the Entity from the spreadsheet
	    if (entity->Requirements.size() < 1) {
		entity->Requirements.push_back(*(new list<LineItem*>));
		entity->Provides.push_back(*(new list<LineItem*>));
	    }
	    if (entity->ProcessedSpreadsheetDefinition != false) {
		// I had an assertion here to trap any entities here that were already processed - but
		// I don't know why.  Why should they necessarily be unprocessed here?  Oh!  because if
		// we are here, then we are processing it.  And we should only process it once!
		cout << "ERROR: we are processing " << fullyQualifiedName << " from " << fn 
		     << " but it appears that this thing already has a processed entity in the system" << endl;
	    }
	    assert(entity->ProcessedSpreadsheetDefinition == false);
	    //cout << ".";
	} else {
	    //cout << "+";
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
	    StoreEntity(fullyQualifiedName, entity);
	}

	// leter I'll consume the secondary, crowd sourced, table that gives the yield
	// for refining stuff
	entity->CreationIncrement = 1;

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
	req->Quantity = atoi(fields[14].c_str());
	entity->Requirements[0].push_back(req);

	// Feat requirement
	string featName = fields[2];
	int featLevel = atoi(fields[3].c_str());

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
	req->Quantity = featLevel;
	entity->Requirements[0].push_back(req);
	
	// now all the components - we only have three per recipe right now but I think there
	// is space in here for four.
	for (int componentOffset = 5; componentOffset < 10; componentOffset += 2) {
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
	    entity->Requirements[0].push_back(req);	    
	}
	
	delete name;
	// cout << "[" << *name << "] <-> [" << fields[15] << "]" << endl;
    }
    

    fin.close();

    return true;
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
