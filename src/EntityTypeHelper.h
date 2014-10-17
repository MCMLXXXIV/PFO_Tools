#ifndef ENTITYTYPEHELPER_H
#define ENTITYTYPEHELPER_H

#include <set>
#include <list>
#include <vector>

#include "HierarchicalId.h"

using namespace std;

class EntityTypeHelper {
 public:
    static EntityTypeHelper* Instance();

    short* GetType(list<string> names);
    bool IsUniversal(short type);
    bool IsRanked(short type);
    bool QuantityIsWholeNumber(short type);
    bool IsType(short typeId, string typeStr);
    int GetMaxEntityId(short* typeCategory);
    string ToIdString(short *type);
    list<string> GetType(short* type);

 private:
    HierarchicalId IdRoot;

    vector<bool> UniversalFlagByLevelOneTypeId;
    vector<bool> RankedFlagByLevelOneTypeId;
    vector<bool> WholeNumberFlagByLevelOneTypeId;
    vector<string> TopLevelTypes;

    static const set<string> AllowedTopLevelTypes;
    static const set<string> UniversalEntityTypes;
    static const set<string> RankedEntityTypes;
    static const set<string> DecimalEntityTypes;

    EntityTypeHelper(){};
    EntityTypeHelper(EntityTypeHelper const&);
    EntityTypeHelper& operator=(EntityTypeHelper const &);
    static EntityTypeHelper* m_pInstance;

};

#endif
