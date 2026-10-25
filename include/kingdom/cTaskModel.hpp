#pragma once

#include "common.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cTaskModelBase.hpp"
#include "cJetTask.hpp"

namespace kingdom
{

class cTaskModel : public cTaskModelBase
{
public:
	class cPBModel : public cKingdomModel
	{
	};

private:
	std::vector<cTaskModel::cPBModel*> m_vecPBModel;

	void Init();

public:
	void Term() override;

	cTaskModel();

	cTaskModel(const string& key, uint option = 0u);

	cTaskModel(Object* data, uint option = 0u);

	cKingdomModel* CreateModel(uint idx = 0u);

	void DeleteModel(int idx = 0);

	cKingdomModel* pModel(uint idx = 0u);

	cTaskModel::cPBModel* pPBModel(uint idx = 0u);

	virtual void UpdateFrame();

	bool Execute(cJetTask* pParam) override;

protected:
	bool baseExecute(cJetTask* pParam);

public:
	void Draw(int nStatus, cJetTask* pParam) override;
};

}
