#include "LineItem.h"
#include "EntityDefinition.h"

LineItem::LineItem(EntityDefinition* entity, double quantity) {
    Entity = entity;
    Quantity = quantity;
}


void LineItem::Dump() {
    printf("%5.2f of %s\n", Quantity, Entity->Name.c_str());
}
