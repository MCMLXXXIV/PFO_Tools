#include "Cost.h"
#include "LineItem.h"

void Cost::Add(LineItem *item) {
    TempCostList.push_back(new LineItem(item->Entity, item->Quantity));
}

void Cost::Dump() {
    list<LineItem*>::iterator itr = TempCostList.begin();

    for (; itr != TempCostList.end(); ++itr) {
	(*itr)->Dump();
    }
}
