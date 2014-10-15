#include "TrackedResources.h"
#include "EntityTypeHelper.h"

using namespace std;

void TrackedResources::SetTracked(list<short*> tracked) {
    int maxTopLevelEntityId = EntityTypeHelper::Instance()->GetMaxEntityId(NULL);
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
	string key = "";
	short *type = *itr;
	for (int idx = 0; type[idx] > 0; ++idx) {
	    if (key.size() < 1) { key += "."; }
	    key += to_string(type[idx]);
	}
	TrackedByInternalTypeKey[key] = true;
    }

    return;
}

bool TrackedResources::IsTracked(short *type) {
    string key = "";
    for (int idx = 0; type[idx] > 0; ++idx) {
	if (key.size() < 1) { key += "."; }
	key += to_string(type[idx]);
	if (NotTrackedByInternalTypeKey[key]) { return false; }
	if (TrackedByInternalTypeKey[key]) { return true; }
    }
    return false;
}
