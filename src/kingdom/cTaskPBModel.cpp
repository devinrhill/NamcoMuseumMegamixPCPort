#include "kingdom/cTaskPBModel.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomModel.hpp"

namespace kingdom
{

void cTaskPBModel::Init()
{
	Model = new cKingdomModel();
}

void cTaskPBModel::Term()
{
	Model->Term();
}

cTaskPBModel::cTaskPBModel()
{
	Init();
}

cTaskPBModel::cTaskPBModel(const string& key, uint option)
{
	Init();
	Model->RegistModel(key, option);
}

cTaskPBModel::cTaskPBModel(uint key, uint option)
{
	Init();
}

cTaskPBModel::cTaskPBModel(const std::vector<byte>& data, uint option)
{
	Init();
}

cTaskPBModel::~cTaskPBModel()
{
}

void cTaskPBModel::IsDrawShadowDepth(bool b)
{
	Model->IsDrawShadowDepth(b);
}

bool cTaskPBModel::Execute(cJetTask* pParam)
{
	return false;
}

void cTaskPBModel::Draw(int nStatus, cJetTask* pParam)
{
}

void cTaskPBModel::SubmitNative()
{
	if (Model != nullptr) Model->UpdateMatrix();
}

bool cTaskPBModel::IsDrawShadowDepth()
{
	return Model->IsDrawShadowDepth();
}

}
