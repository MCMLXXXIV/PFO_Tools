#include <cassert>

#include <iostream>
#include <vector>
#include <string>

#include "EntityTypeHelper.h"
#include "OfficialData.h"
#include "Planners.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "Utils.h"
#include "CommandLineOptions.h"

void TestUtilsSplit();
void TestSplitKeyValueOnChar();
void TestRankSpliter();
void DumpItemRequirements(string items_arg);
void GetPlanForItems(string items_arg);
void SearchForItemsThatRequire(string items_arg);

using namespace std;

int main(int argc, char **argv) {

    // TestUtilsSplit(); return 0;
    // TestRankSpliter() ; return 0;

    CommandLineOptionsEncapsulation opts;
    opts.ParseArgs(argc, argv);

    if (opts.ParseError) {
	cout << opts.ErrMsg << endl;
    }
    if (opts.ParseError || opts.ShowHelpOpt) {
	opts.ShowHelp();
	return 0;
    }

    OfficialData* rulesGraph = OfficialData::Instance();
    rulesGraph->ProcessSpreadsheetDir("official_data");
    
    if (opts.DumpItems) {
	rulesGraph->Dump();
	return 0;
    }

    if (opts.DumpItemReqs) {
	DumpItemRequirements(opts.Items);
	return 0;
    }

    if (opts.GetPlanForItem) {
	GetPlanForItems(opts.Items);
	return 0;
    }

    if (opts.SearchForItemsThatRequire) {
	SearchForItemsThatRequire(opts.Items);
	return 0;
    }

    cout << "syntax error: no options" << endl;
    opts.ShowHelp();
    return 0;

    cout << argc << endl;
    return 0;


    EntityDefinition *entity = NULL;

    // TestSplitKeyValueOnChar(); return 0;
    
    // return 0;

    // 
    entity = rulesGraph->GetEntity("Skill.Armorsmith");
    assert(entity != NULL);
    // cout << EntityDefinition::Dump(*entity) << endl; return 0;

    EntityDefinition *headEntity = new EntityDefinition();
    headEntity->Name = "LogicAnd";
    list<string> typeStringName;
    typeStringName.push_back("LogicAnd");

    headEntity->Type = EntityTypeHelper::Instance()->GetType(typeStringName);

    // entity = rulesGraph->GetEntity("Item.Pot Steel Plate");
    // entity = rulesGraph->GetEntity("Item.Journeyman's Speed Potion");
    
    headEntity->Requirements.push_back(list<LineItem*>());

    //entity = rulesGraph->GetEntity("Item.Hunter's Longbow");
    entity = rulesGraph->GetEntity("Adept's Spellbook");
    assert(entity != NULL);

    // cout << EntityDefinition::Dump(*entity) << endl; return 0;

    LineItem *stuff = new LineItem(entity, 2.0);
    headEntity->Requirements[0].push_back(stuff);

    entity = rulesGraph->GetEntity("Item.Yew and Iron Splint");
    assert(entity != NULL);
    stuff = new LineItem(entity, 1.0);
    //headEntity->Requirements[0].push_back(stuff);

    LineItem *headGoal = new LineItem(headEntity, 1.0);

    Supply bank;

    TrackedResources trackedResources;

    list<short*> trackedTypes;

    typeStringName.clear();
    typeStringName.push_back("Item");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("Skill");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("Time");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    typeStringName.clear();
    typeStringName.push_back("ExperiencePoint");
    trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));

    trackedResources.SetTracked(trackedTypes);

    Cost cost;
    Supply remainder;
    //Plan *plan =
    Planners::CreatePlanForItemsGoal(headGoal, bank, trackedResources, cost);

    cout << endl;
    cout << "Cost:" << endl;
    cost.Dump();
    cout << endl;

    cout << "Final Supply:" << endl;
    bank.Dump();
    cout << endl;

    cout << endl;
    cout << EntityDefinition::Dump(*entity, 1.0) << endl; return 0;
    return 0;
}

void DumpItemRequirements(string itemsArg) {
    vector<string> itemsVec = Utils::SplitCommaSeparatedValuesWithQuotedFields(itemsArg.c_str());
    if (itemsVec.size() == 0) {
	return;
    }

    if (itemsVec.size() == 1) {
	EntityDefinition *entity = NULL;

	string entityName;
	int rankInName;
	char *namePart;
	if (Utils::RankInName(itemsVec[0].c_str(), &namePart, rankInName)) {
	    entityName = namePart;
	    delete namePart;
	    // headGoal->Quantity = rankInName * 1.0;	    
	} else {
	    entityName = itemsVec[0];
	    rankInName = -1;
	}

	entity = OfficialData::Instance()->GetEntity(entityName);
	if (entity == NULL) {
	    cout << "can't find this item: [" << itemsVec[0] << "]" << endl;
	    return;
	}

	cout << EntityDefinition::Dump(*entity, (rankInName * 1.0)) << endl;
    } else {
	cout << "haven't implemented this feature for multiple items" << endl;
    } 

    return;
}

void GetPlanForItems(string itemsArg) {
    vector<string> itemsVec = Utils::SplitCommaSeparatedValuesWithQuotedFields(itemsArg.c_str());
    if (itemsVec.size() == 0) {
	return;
    }

    if (itemsVec.size() == 1) {
	EntityDefinition *entity = NULL;

	LineItem *headGoal = new LineItem(entity, 1.0);

	string entityName;
	int rankInName;
	char *namePart;
	if (Utils::RankInName(itemsVec[0].c_str(), &namePart, rankInName)) {
	    entityName = namePart;
	    delete namePart;
	    headGoal->Quantity = rankInName * 1.0;	    
	} else {
	    entityName = itemsVec[0];
	}

	entity = OfficialData::Instance()->GetEntity(entityName);
	if (entity == NULL) {
	    cout << "can't find this item: [" << itemsVec[0] << "]" << endl;
	    return;
	}
	headGoal->Entity = entity;

	Cost cost;
	Supply remainder;
	Supply bank;
	TrackedResources trackedResources;
 
	list<string> typeStringName;
	list<short*> trackedTypes;
	typeStringName.clear();
	typeStringName.push_back("Item");
	trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));
	
	typeStringName.clear();
	typeStringName.push_back("Skill");
	trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));
	
	typeStringName.clear();
	typeStringName.push_back("Time");
	trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));
	
	typeStringName.clear();
	typeStringName.push_back("ExperiencePoint");
	trackedTypes.push_back(EntityTypeHelper::Instance()->GetType(typeStringName));
	
	trackedResources.SetTracked(trackedTypes);
	trackedResources.DumpTrackedResources();

	Planners::CreatePlanForItemsGoal(headGoal, bank, trackedResources, cost);
	
	cout << endl;
	cout << "Cost:" << endl;
	cost.Dump();
	cout << endl;
	
	cout << "Final Supply:" << endl;
	bank.Dump();
	cout << endl;
    } else {
	cout << "haven't implemented this feature for multiple items" << endl;	
    }
    return;
}

void SearchForItemsThatRequire(string itemsArg) {
    vector<string> itemsVec = Utils::SplitCommaSeparatedValuesWithQuotedFields(itemsArg.c_str());
    if (itemsVec.size() == 0) {
	return;
    }

    if (itemsVec.size() == 1) {
	EntityDefinition *entity = NULL;
	entity = OfficialData::Instance()->GetEntity(itemsVec[0]);
	if (entity == NULL) {
	    cout << "can't find this item: [" << itemsVec[0] << "]" << endl;
	    return;
	}
	OfficialData::Instance()->SearchForItemsThatRequire(entity);
    } else {
	cout << "haven't implemented this feature for multiple items" << endl;
    }

    return;
}



void TestSplitKeyValueOnChar() {
    vector<string> tvalsA = {string("martial=6"), string(""), string(" craft = 4"), string("foo=  "), string("bar =6 "), string(" =7"), string("=8")};
    vector<string> tvalsB = {string("martial or 6"), string(""), string(" craft  or  4"), string("foo or  "), string("bar  or 6 "), string(" or 7"), string(" or 8")};

    vector<string>::iterator itr = tvalsA.begin();
    for(; itr != tvalsA.end(); ++itr) {
	string key, val;
	bool retVal = Utils::SplitKeyValueOnChar((*itr).c_str(), "=", key, val);
	if (retVal == true) {
	    printf("%13s -> TRUE, [%s] [%s]\n", (*itr).c_str(), key.c_str(), val.c_str());
	} else {
	    printf("%13s -> FALS\n", (*itr).c_str());
	}
    }

    cout << endl;

    itr = tvalsB.begin();
    for(; itr != tvalsB.end(); ++itr) {
	string key, val;
	bool retVal = Utils::SplitKeyValueOnChar((*itr).c_str(), " or ", key, val);
	if (retVal == true) {
	    printf("%13s -> TRUE, [%s] [%s]\n", (*itr).c_str(), key.c_str(), val.c_str());
	} else {
	    printf("%13s -> FALS\n", (*itr).c_str());
	}
    }

}


void TestUtilsSplit() {
    
    vector<string> foo = Utils::SplitCommaSeparatedValuesWithQuotedFields("foo,bar ,\"choo,crab\", ,blue, green, \"in, out \", red \"and, this\", "" , \"rog,");
    vector<string>::iterator fItr;
    int idx = 0;
    for (fItr = foo.begin(); fItr != foo.end(); ++fItr) {
	cout << idx << " [" << *fItr << "]" << endl;
	++idx;
    }

    foo = Utils::SplitCommaSeparatedValuesWithQuotedFields("Willpower Bonus=7, Arcane Attack Bonus=7, Power=26,");
    idx = 0;
    for (fItr = foo.begin(); fItr != foo.end(); ++fItr) {
	cout << idx << " [" << *fItr << "]" << endl;
	++idx;
    }

    return;
}


void TestRankSpliter() {
    vector<string> foo = {"Pine Pole", "Pine Pole +1", "Fighter 8", "Fighter Level 6"};
    vector<string>::iterator itr;
    for (itr = foo.begin(); itr != foo.end(); ++itr) {
	int rankInName = -1;
	char *namePart;
	bool ret = Utils::RankInName((*itr).c_str(), &namePart, rankInName);
	printf("%15s %5s [%s] [%d]\n", (*itr).c_str(), (ret ? "TRUE" : "FALSE"), (ret ? namePart : "n/a"), rankInName);
    }
} 
