#ifndef LINEITEM_H
#define LINEITEM_H

class EntityDefinition;

class LineItem {
 public:
    LineItem(){};
    LineItem(EntityDefinition* entity, double quantity);

    EntityDefinition* Entity;
    double Quantity;
};

#endif
