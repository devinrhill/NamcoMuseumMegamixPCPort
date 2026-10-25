#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskModelEx.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskModel.hpp"

namespace kingdom::game::g03
{

class cTaskCharBEye : public cTaskModelEx
{
public:
	cTaskCharBEye(cTaskModel* pParent);

	void Term() override;

	void StartEye(const Vector4& pos);

	bool IsEndEye();

	static cTaskCharBEye* Create(cTaskModel* pParent);

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
