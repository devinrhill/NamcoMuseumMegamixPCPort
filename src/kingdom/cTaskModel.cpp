#include "kingdom/cTaskModel.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"

namespace kingdom
{

void cTaskModel::Init()
{
	m_vecPBModel.reserve(32);
	SetName("cTaskModel");
}

void cTaskModel::Term()
{
	EnableGarbageModel();
	ClearGarbageModel();
	for (int i = 0; i < (int)m_vecPBModel.size(); i++)
	{
		if (m_vecPBModel[i] != nullptr) m_vecPBModel[i]->Term();
	}
	m_vecPBModel.clear();
}

cTaskModel::cTaskModel()
{
	Init();
}

cTaskModel::cTaskModel(const string& key, uint option)
{
	Init();
	CreateModel()->RegistModel(key, option);
}

cTaskModel::cTaskModel(Object* data, uint option)
{
	Init();
	CreateModel()->RegistModel(data, option);
}

cKingdomModel* cTaskModel::CreateModel(uint idx)
{
	cTaskModel::cPBModel* cPBModel2 = new cTaskModel::cPBModel();
	CustomKingdomUtil::Assert(cPBModel2 != nullptr, "nullpo");
	if ((long)(int)m_vecPBModel.size() <= (long)idx)
	{
		int num = (int)(idx + 1) - (int)m_vecPBModel.size();
		for (int i = 0; i < num; i++)
		{
			m_vecPBModel.push_back(nullptr);   // the gap slots stay empty (the converted code filled them all with this model)
		}
	}
	m_vecPBModel[(int)idx] = cPBModel2;
	return m_vecPBModel[(int)idx];
}

void cTaskModel::DeleteModel(int idx)
{
	CustomKingdomUtil::Assert((int)m_vecPBModel.size() > idx, "error");
	(void)(m_vecPBModel[idx]);
}

cKingdomModel* cTaskModel::pModel(uint idx)
{
	if ((long)(int)m_vecPBModel.size() <= (long)idx)
	{
		return nullptr;
	}
	return m_vecPBModel[(int)idx];
}

cTaskModel::cPBModel* cTaskModel::pPBModel(uint idx)
{
	if ((long)(int)m_vecPBModel.size() <= (long)idx)
	{
		return nullptr;
	}
	return m_vecPBModel[(int)idx];
}

void cTaskModel::UpdateFrame()
{
	cTaskModelBase::UpdateFrame(m_vecPBModel);   // the converted code called itself (overload lost)
}

bool cTaskModel::Execute(cJetTask* pParam)
{
	return cTaskModelBase::Execute(m_vecPBModel);
}

bool cTaskModel::baseExecute(cJetTask* pParam)
{
	return cTaskModelBase::Execute(m_vecPBModel);
}

void cTaskModel::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModelBase::Draw(m_vecPBModel);
}

}
