#ifndef COSTNODE_H
#define COSTNODE_H

class CostNode {
 public:
    map<short, CostNode*> SubNodes;
    double Sum;
};

#endif
