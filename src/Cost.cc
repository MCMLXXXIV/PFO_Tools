#include "Cost.h"
#include "LineItem.h"

void Cost::Add(LineItem *item) {
    TempCostList.push_back(new LineItem(item->Entity, item->Quantity));
}
