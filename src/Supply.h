#ifndef SUPPLY_H
#define SUPPLY_H

#include <string>
#include <map>

class EntityDefinition;
class LineItem;

using namespace std;

class Supply {
 public:
    map<string, LineItem*> Items;
    // bool Has(EntityDefinition *entity);
    double Withdrawal(LineItem *item);
    void Deposit(LineItem *item);
    void Dump();
};


#endif
