#ifndef PROVIDESNODE_H
#define PROVIDESNODE_H

#include "EntityDefinition.h"

enum ProvidesType { AbilityScoreChange, AchievementPoints };

class ProvidesNode {
 public:
    ProvidesType Type;
    EntityDefinition* Entity;
    union QuantityValueType {
	long _lval;
	double _dval;
    } QuantityValue;

    long Quantity_long();
    double Quantity_double();
};

#endif
