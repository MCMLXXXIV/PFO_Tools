#include "TrackedResources.h"
#include "EntityTypeHelper.h"
#include "Log.h"

#include <iostream>

using namespace std;

void TrackedResources::SetTracked(list<short*> tracked) {
    EntityTypeHelper *h = EntityTypeHelper::Instance();
    Logger *log = Logger::Instance();

    int maxTopLevelEntityId = h->GetMaxEntityId(NULL);
    set<short> trackedTopLevelTypes;
    list<short*>::iterator itr;
    for (itr = tracked.begin(); itr != tracked.end(); ++itr) {
	if ((*itr) != NULL && **itr != 0) {
	    trackedTopLevelTypes.insert(**itr);
	}
    }
    
    for (int topLevelEntityId = 0; topLevelEntityId < maxTopLevelEntityId; ++topLevelEntityId) {
	if (trackedTopLevelTypes.count((short)topLevelEntityId) == 0) {
	    NotTrackedByInternalTypeKey[to_string(topLevelEntityId)] = true;
	}
    }

    for (itr = tracked.begin(); itr != tracked.end(); ++itr) {
	string key = h->ToIdString(*itr);
	log->Log(Logger::Level::Verbose, "TrackedResources", "adding %s: %s\n", h->GetTypePrettyString(*itr).c_str(), key.c_str());
	TrackedByInternalTypeKey[key] = true;
    }

    // also, logicAnd and logicOr nodes are tracked always
    list<string> typeName = { "LogicAnd" };
    short *logicGate = h->GetType(typeName);
    string typeStr = h->ToIdString(logicGate);
    log->Log(Logger::Level::Verbose, "TrackedResources", "adding %s: %s\n", typeName.front().c_str(), typeStr.c_str());
    TrackedByInternalTypeKey[typeStr] = true;
    NotTrackedByInternalTypeKey.erase(typeStr);

    typeName.clear();
    typeName.push_back("LogicOr");
    logicGate = h->GetType(typeName);
    typeStr = h->ToIdString(logicGate);
    log->Log(Logger::Level::Verbose, "TrackedResources", "adding %s: %s\n", typeName.front().c_str(), typeStr.c_str());
    TrackedByInternalTypeKey[typeStr] = true;
    NotTrackedByInternalTypeKey.erase(typeStr);

    return;
}

bool TrackedResources::IsTracked(short *type) {
    string key = "";
    for (int idx = 0; type[idx] > 0; ++idx) {
	if (key.size() > 1) { key += "."; }
	key += to_string(type[idx]);
	if (NotTrackedByInternalTypeKey[key]) { return false; }
	if (TrackedByInternalTypeKey[key]) { return true; }
    }
    return false;
}

void TrackedResources::DumpTrackedResources() {
    cout << "Tracked:" << endl;
    map<string, bool>::iterator itr = TrackedByInternalTypeKey.begin();
    for (; itr != TrackedByInternalTypeKey.end(); ++itr) {
	cout << (*itr).first << endl;
    }
    cout << endl << "NotTracked:" << endl;
    itr = NotTrackedByInternalTypeKey.begin();
    for (; itr != NotTrackedByInternalTypeKey.end(); ++itr) {
	cout << (*itr).first << endl;
    }
}
