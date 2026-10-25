#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"

namespace kingdom::game::g07
{

class cTaskConsPlayTime : public cTaskConsoleBase
{
private:
	enum EItem
	{
		eITEM_MINUTE = 0,
		eITEM_SECOND = 1,
		eITEM_MILLI = 2,
		eITEM_MAX = 3
	};

	enum EAnt
	{
		eANT_CANMA1 = 0,
		eANT_CANMA2 = 1
	};

	uint m_nTimer{};

	bool m_bGo{};

	Vector4 m_vOffset{};

	cTaskConsItemNumber* m_pItemNumbers[3]{};

	Ref<int>* m_aExecNumbers[3]{};

public:
	cTaskConsPlayTime();

	virtual ~cTaskConsPlayTime();

	void Free() override;

	Vector4& vOffset();

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
