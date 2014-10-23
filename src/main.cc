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

void TestSplitKeyValueOnChar();

using namespace std;

int main() {

    // TestSplitKeyValueOnChar(); return 0;
    
    OfficialData rulesGraph;
    rulesGraph.ProcessSpreadsheetDir("official_data");

    // rulesGraph.Dump(); return;

    EntityDefinition *headEntity = new EntityDefinition();
    headEntity->Name = "LogicAnd";
    list<string> typeStringName;
    typeStringName.push_back("LogicAnd");

    headEntity->Type = EntityTypeHelper::Instance()->GetType(typeStringName);

    // entity = rulesGraph.GetEntity("Pot Steel Plate");
    // entity = rulesGraph.GetEntity("Journeyman's Speed Potion");
    
    headEntity->Requirements.push_back(list<LineItem*>());

    EntityDefinition *entity = rulesGraph.GetEntity("Hunter's Longbow");
    assert(entity != NULL);
    LineItem *stuff = new LineItem(entity, 2.0);
    headEntity->Requirements[0].push_back(stuff);

    entity = rulesGraph.GetEntity("Yew and Iron Splint");
    assert(entity != NULL);
    stuff = new LineItem(entity, 1.0);
    headEntity->Requirements[0].push_back(stuff);

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
    Plan *plan;
    plan = Planners::CreatePlanForItemsGoal(headGoal, bank, trackedResources, cost);

    cout << endl;
    cout << "Cost:" << endl;
    cost.Dump();
    cout << endl;

    cout << "Final Supply:" << endl;
    bank.Dump();
    cout << endl;

    return 0;
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

    vector<string> foo = Utils::SplitCommaSeparatedValuesWithQuotedFields("foo,bar ,\"choo,crab\", ,blue, green, \"in, out \", red \"and, this\", "" , \"rog ");
    vector<string>::iterator fItr;
    int idx = 0;
    for (fItr = foo.begin(); fItr != foo.end(); ++fItr) {
	cout << idx << " [" << *fItr << "]" << endl;
	++idx;
    }
    return;
}
