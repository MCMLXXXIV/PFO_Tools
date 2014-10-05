#include <cassert>

#include "RequirementNode.h"

long RequirementNode::Quantity_long() {
    assert(this->Type != ScoreRequirement);
    return this->QuantityValue._lval;
}

double RequirementNode::Quantity_double() {
    assert(this->Type == ScoreRequirement);
    return this->QuantityValue._dval;
}
