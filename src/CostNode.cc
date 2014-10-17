#include "CostNode.cc"

void CostNode::Dump(string indent) {

    map<short, CostNode*>::iterator itr = SubNodes->begin();
    for (; itr != subNodes->end(); ++itr) {
	list<string> type = EntityTypeHelper::Instance()->GetType((*itr).second->Type);
	cout << indent << type.front() << ": " << (*itr).second->Sum;
	(*itr).second->Dump(indent + "    ");	
    }
}
