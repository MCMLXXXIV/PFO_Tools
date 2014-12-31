#ifndef SUPPLY_H
#define SUPPLY_H

#include <string>
#include <map>
#include <vector>

class EntityDefinition;
class LineItem;

using namespace std;

class Supply {
 public:
    map<string, vector<LineItem*> > Items;
    bool Withdrawal(LineItem *item);
    void Deposit(LineItem *item);
    void Dump();
    static Supply* Deserialize(const char*);
    static string SerializeJson(Supply *supply);
};


#endif
