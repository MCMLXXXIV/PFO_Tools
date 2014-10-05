#include <cassert>

#include "ProvidesNode.h"

long ProvidesNode::Quantity_long() {
    assert(this->Type != AbilityScoreChange);
    return this->QuantityValue._lval;
}

double ProvidesNode::Quantity_double() {
    assert(this->Type != AchievementPoints);
    return this->QuantityValue._dval;
}
