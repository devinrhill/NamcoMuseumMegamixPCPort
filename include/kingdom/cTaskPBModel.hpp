#pragma once

#include "common.hpp"
#include "kingdom/cTaskPushBack.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomModel.hpp"

namespace kingdom
{

class cTaskPBModel : public cTaskPushBack
{
public:
	cKingdomModel* Model{};

private:
	void Init();

public:
	virtual void Term();

	cTaskPBModel();

	cTaskPBModel(const string& key, uint option = 0u);

	cTaskPBModel(uint key, uint option = 0u);

	cTaskPBModel(const std::vector<byte>& data, uint option = 0u);

	virtual ~cTaskPBModel();

	void IsDrawShadowDepth(bool b);

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool IsDrawShadowDepth() override;

protected:
	void SubmitNative() override;
};

}
