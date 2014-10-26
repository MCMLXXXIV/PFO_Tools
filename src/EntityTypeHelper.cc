#include "EntityTypeHelper.h"

#include <iostream>
#include <climits>
#include <cassert>

// Perception is listed as a skill in Skills Advancement.csv but is refered to as a
// Feat in Utility Advancement.csv (in one of the "Feat LvX" columns.

// Strength is two entities: an AbilityScore and an Expendable (Expendables Advancement.csv)

const string aTypes[] = {"AbilityScore","Achievement","AchievementPoint","ExperiencePoint","Feat","Item","LogicAnd","LogicOr","Recipe","Skill","Time"};
const string uTypes[] = {"Achievement","Feat","Skill","AchievementPoint","AbilityScore"};
const string rTypes[] = {"Achievement","Feat","Skill"};
const string fTypes[] = {"AbilityScore"};
const set<string> EntityTypeHelper::AllowedTopLevelTypes (aTypes, aTypes + sizeof(aTypes)/sizeof(aTypes[0]));
const set<string> EntityTypeHelper::UniversalEntityTypes (uTypes, uTypes + sizeof(uTypes)/sizeof(uTypes[0]));
const set<string> EntityTypeHelper::RankedEntityTypes (rTypes, rTypes + sizeof(rTypes)/sizeof(rTypes[0]));
const set<string> EntityTypeHelper::DecimalEntityTypes (fTypes, fTypes + sizeof(fTypes)/sizeof(fTypes[0]));

//const set<string> EntityTypeHelper::UniversalEntityTypes {"Achievement","Feat","Skill","AchievementPoint","AbilityScore"};
//const set<string> EntityTypeHelper::RankedEntityTypes {"Achievement","Feat","Skill"};

EntityTypeHelper* EntityTypeHelper::m_pInstance = NULL;

EntityTypeHelper* EntityTypeHelper::Instance() {
    if (!m_pInstance) {
	m_pInstance = new EntityTypeHelper();
    }
    return m_pInstance;
}

bool EntityTypeHelper::IsType(short typeId, string typeName) {
    bool givenTypeIsAllowed = (AllowedTopLevelTypes.count(typeName) > 0);
    assert(givenTypeIsAllowed == true);
    
    return (typeName == TopLevelTypes[typeId]);    
}

EntityTypeHelper::EntityTypeHelper() {
    // I'm sure this is a hack but I don't care... :)
    // I need to add time here so no one else has to - and if no one adds it, then the
    // method QuantityIsWholeNumber() won't return true for time.
    list<string> timeType = {string("Time")};
    GetType(timeType);
}

bool EntityTypeHelper::QuantityIsWholeNumber(short type) {
    return WholeNumberFlagByLevelOneTypeId[type];
}

short* EntityTypeHelper::GetType(list<string> names) {
    HierarchicalId *hierId = &IdRoot;
    list<short> ids;
    list<string>::iterator nameListEntry = names.begin();
    for (; nameListEntry != names.end(); nameListEntry++) {
	string name = *nameListEntry;
	if (hierId->NextLevel[name] == NULL) {
	    // there are two things going on here:
	    // 1) the check above, "if map[s] != NULL", causes the entry for s to be created
	    // 2) the zero value will be used to flag the end of the id chain so we want to skip it here
	    uint nextId = hierId->NextLevel.size();
	    assert(nextId < SHRT_MAX);
	    if (ids.size() < 1) {
		// if ids.size < 1 then we are processing the top level type

		if (UniversalFlagByLevelOneTypeId.size() == 0) { UniversalFlagByLevelOneTypeId.push_back(false); }
		if (RankedFlagByLevelOneTypeId.size() == 0) { RankedFlagByLevelOneTypeId.push_back(false); }
		if (WholeNumberFlagByLevelOneTypeId.size() == 0) { WholeNumberFlagByLevelOneTypeId.push_back(false); }
		if (TopLevelTypes.size() == 0) { TopLevelTypes.push_back(""); }
		cout << "name: " << name << "; nextId: " << nextId << "; vectorSize: " << UniversalFlagByLevelOneTypeId.size() << endl;
		assert(UniversalFlagByLevelOneTypeId.size() == nextId);
		UniversalFlagByLevelOneTypeId.push_back(UniversalEntityTypes.count(name) > 0);
		RankedFlagByLevelOneTypeId.push_back(RankedEntityTypes.count(name) > 0);
		WholeNumberFlagByLevelOneTypeId.push_back(DecimalEntityTypes.count(name) < 1);
		TopLevelTypes.push_back(name);
	    }
	    ids.push_back(nextId);
	    HierarchicalId *newHierLevel = new HierarchicalId(name, nextId);
	    hierId->NextLevel[name] = newHierLevel;
	    hierId = newHierLevel;
	} else {
	    ids.push_back(hierId->NextLevel[name]->Index);
	    HierarchicalId *newHierLevel = hierId->NextLevel[name];
	    hierId = newHierLevel;
	}
    }

    short *retVal = new short[ids.size()+1];
    int idx = 0;
    list<short>::iterator idListEntry = ids.begin();
    for (; idListEntry != ids.end(); idListEntry++, ++idx) {
	retVal[idx] = *idListEntry;
    }
    retVal[ids.size()] = 0;

    return retVal;
}

bool EntityTypeHelper::IsUniversal(short type) {
    return UniversalFlagByLevelOneTypeId[type];
}

bool EntityTypeHelper::IsRanked(short type) {
    return RankedFlagByLevelOneTypeId[type];
}

int EntityTypeHelper::GetMaxEntityId(short* typeCategory) {
    HierarchicalId *idsList = &IdRoot;
    if (typeCategory != NULL) {
	int categoryIndex = 0;
	while (typeCategory[categoryIndex] != 0) {
	    HierarchicalId *nextIdList = NULL;
	    map<string, HierarchicalId*>::iterator itr = idsList->NextLevel.begin();
	    for (; itr != idsList->NextLevel.end(); ++itr) {
		if ((*itr).second->Index == (int)typeCategory[categoryIndex]) {
		    nextIdList = (*itr).second;
		    break;
		}
	    }
	    if (itr != idsList->NextLevel.end()) {
		// they asked for an id that is out of bounds
		return -1;
	    }
	    idsList = nextIdList;
	    ++categoryIndex;
	}
    }
    return idsList->NextLevel.size();
}

string EntityTypeHelper::ToIdString(short *type) {
    string key = "";
    if (type == NULL) { return ""; }

    for (int idx = 0; type[idx] > 0; ++idx) {
	if (key.size() > 0) { key += "."; }
	key += to_string(type[idx]);
    }
    return key;
}

list<string> EntityTypeHelper::GetType(short* type) {
    list<string> retVal;
    if (type == NULL) { return retVal; }

    map<string, HierarchicalId*> *subIds = &(IdRoot.NextLevel);
    while (*type != 0 && subIds != NULL) {
	// this is ugly - need to make a map for this
	map<string, HierarchicalId*> *nextSubIdLevel = NULL;
	map<string, HierarchicalId*>::iterator itr = subIds->begin();
	for (; itr != subIds->end(); ++itr) {
	    if ( (*itr).second->Index == *type ) {
		retVal.push_back((*itr).first);
		++type;
		nextSubIdLevel = &( (*itr).second->NextLevel );
		break;
	    }
	}
	subIds = nextSubIdLevel;
    }
    return retVal;
}
