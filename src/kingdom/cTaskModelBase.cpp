#include "kingdom/cTaskModelBase.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskPushBack.hpp"

namespace kingdom
{

void cTaskModelBase::Init()
{
	m_vecGarbage.reserve(32);
	SetName("cTaskModelBase");
}

void cTaskModelBase::Term()
{
}

void cTaskModelBase::GarbageModel(cTaskPushBack* pb)
{
	if (pb != nullptr)
	{
		cTaskModelBase::sGarbage* sGarbage2 = new cTaskModelBase::sGarbage();
		sGarbage2->pb = pb;
		sGarbage2->isGarbage = false;
		m_vecGarbage.push_back(sGarbage2);
	}
}

void cTaskModelBase::ClearGarbageModel()
{
	for (cTaskModelBase::sGarbage* item : m_vecGarbage)
	{
		(void)(((item != nullptr) ? item->isGarbage : false));
	}
}

void cTaskModelBase::EnableGarbageModel()
{
	for (cTaskModelBase::sGarbage* item : m_vecGarbage)
	{
		if (item != nullptr)
		{
			item->isGarbage = true;
		}
	}
}

cTaskModelBase::cTaskModelBase()
{
	Init();
}

bool cTaskModelBase::Execute(cJetTask* pParam)
{
	return false;
}

void cTaskModelBase::Draw(int nStatus, cJetTask* pParam)
{
}

}
