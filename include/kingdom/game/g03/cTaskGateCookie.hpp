#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskGateCookie : public cTaskModel
{
private:
	uint m_nCnt{};

	bool m_bBreak{};

public:
	void SetCnt(uint n);

	cTaskGateCookie(const Vector4& trans);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Invisible();
};

}
