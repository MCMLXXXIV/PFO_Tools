#ifndef SUPPLY_H
#define SUPPLY_H

#include <string>
#include <list>

class EntityDefinition;
class LineItem;

using namespace std;

class Supply {
 public:
    list<EntityDefinition> Items;
    bool Has(EntityDefinition *entity);
    double Withdrawal(LineItem *item);
    void Deposit(LineItem *item);
};


#endif
