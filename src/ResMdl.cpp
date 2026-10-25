#include "ResMdl.hpp"

#include "RMDLUserDataParam.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "kingdom/cKingdomModel.hpp"

// Native ResMdl (original Unity version in old/BACKUP_unity_headers/ResMdl.cpp.orig): the node tree of the model's parsed MDL0. The Unity hierarchy had the
// model's root node as the first child of the GameObject, so "entries" are the children of node 0 and "child entries" the children of its first child.
ResMdl::ResMdl()
{
}

ResMdl::ResMdl(GameObject* baseObj)
{
	This = baseObj;
	if (baseObj != nullptr && baseObj->native != nullptr) Mdl = static_cast<kingdom::cKingdomModel*>(baseObj->native)->pMdl0();
}

bool ResMdl::IsValid()
{
	return nullptr != This;
}

namespace
{
int NthChild(const wii::Mdl0* m, int parent, int n)
{
	int i = 0;
	for (const auto& c : m->nodes)
		if (c.parent == parent && i++ == n) return c.id;
	return -1;
}
int ChildCount(const wii::Mdl0* m, int parent)
{
	int i = 0;
	for (const auto& c : m->nodes) if (c.parent == parent) i++;
	return i;
}
}

ResNode* ResMdl::GetResNode(string nodeName)
{
	if (Mdl != nullptr)
		for (const auto& n : Mdl->nodes)
			if (n.name == nodeName) return new ResNode(Mdl, n.id);
	return new ResNode();
}

ResNode* ResMdl::GetResNode(int idx)
{
	if (Mdl == nullptr || Mdl->nodes.empty()) return new ResNode();
	const int c = NthChild(Mdl, 0, idx);
	return c >= 0 ? new ResNode(Mdl, c) : new ResNode();
}

ResNode* ResMdl::GetFirstChildResNode(int idx)
{
	if (Mdl == nullptr || Mdl->nodes.empty()) return new ResNode();
	const int first = NthChild(Mdl, 0, 0);
	const int c = first >= 0 ? NthChild(Mdl, first, idx) : -1;
	return c >= 0 ? new ResNode(Mdl, c) : new ResNode();
}

ResNode* ResMdl::GetFirstChildChildResNode(int idx)
{
	if (Mdl == nullptr || Mdl->nodes.empty()) return new ResNode();
	const int first = NthChild(Mdl, 0, 0);
	const int second = first >= 0 ? NthChild(Mdl, first, 0) : -1;
	const int c = second >= 0 ? NthChild(Mdl, second, idx) : -1;
	return c >= 0 ? new ResNode(Mdl, c) : new ResNode();
}

int ResMdl::GetResNodeNumEntries()
{
	return (Mdl != nullptr && !Mdl->nodes.empty()) ? ChildCount(Mdl, 0) : 0;
}

int ResMdl::GetChildResNodeNumEntries()
{
	if (Mdl == nullptr || Mdl->nodes.empty()) return 0;
	const int first = NthChild(Mdl, 0, 0);
	return first >= 0 ? ChildCount(Mdl, first) : 0;
}

void ResMdl::SetNodeUserData(ResNode* node)
{
}
