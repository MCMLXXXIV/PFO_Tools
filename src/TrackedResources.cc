#include "TrackedResources.h"
#include "EntityTypeHelper.h"

void TrackedResources::SetTracked(list<short[]> tracked) {
    int maxTopLevelEntityId = EntityTypeHelper::Instance()->GetMaxEntityId(NULL);
    set<short> trackedTopLevelTypes;
    list<short[]>::iterator itr;
    for (itr = tracked.begin(); itr != tracked.end(); ++itr) {
	//EntityTypeHelper::Instance()->
    }

    // TODO
    foreach (type in tracked) {
	string key = "";
	for (int idx = 0; type[idx] > 0; ++idx) {
	    if (key.size() < 1) { key += "."; }
	    key += itoa(type[idx]);
	}
	IsTrackedMap[key] = true;
    }

    return;
}




bool TrackedResources::IsTracked(short *type) {

    // TODO
    string key = "";
    for (int idx = 0; type[idx] > 0; ++idx) {
	if (key.size() < 1) { key += "."; }
	key += itoa(type[idx]);
	if (NotTrackedMap[key]) { return false; }
	if (IsTrackedMap[key]) { return true; }
    }
    return false;
}
