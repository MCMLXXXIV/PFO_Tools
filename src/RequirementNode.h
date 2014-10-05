#ifndef REQUIREMENTNODE_H
#define REQUIREMENTNODE_H

#include "EntityDefinition.h"

enum RequirementType { Entity, PointRequirement, ScoreRequirement, Time, ExperiencePoints, LogicAnd, LogicOr };

class RequirementNode {
 public:
    RequirementType Type;
    EntityDefinition* Entity;
    union QuantityValueType {
	long _lval;
	double _dval;
    } QuantityValue;
    
    long Quantity_long();
    double Quantity_double();
};

#endif
