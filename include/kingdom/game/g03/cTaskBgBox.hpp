#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/sSaveDataBox.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgBox : public cTaskBgHit
{
public:
	enum EKind
	{
		eKIND_WOOD_L = 0,
		eKIND_WOOD_S = 1,
		eKIND_IRON_L = 2,
		eKIND_IRON_S = 3,
		eKIND_MAX = 4
	};

	enum ETreasure
	{
		eTREASURE_NONE = 0,
		eTREASURE_COOKIE_1 = 1,
		eTREASURE_COOKIE_3 = 2,
		eTREASURE_COOKIE_6 = 3,
		eTREASURE_COOKIE_9 = 4,
		eTREASURE_COOKIE_POW = 5,
		eTREASURE_CHOCO_KNIGHT = 6,
		eTREASURE_CHOCO_WING = 7,
		eTREASURE_CHOCO_PACN = 8,
		eTREASURE_APPLE_L = 9,
		eTREASURE_APPLE_S = 10,
		eTREASURE_GRAPE_L = 11,
		eTREASURE_GRAPE_S = 12,
		eTREASURE_BANANA_L = 13,
		eTREASURE_BANANA_S = 14,
		eTREASURE_HAMBURGER_L = 15,
		eTREASURE_HAMBURGER_S = 16,
		eTREASURE_PARFAIT_L = 17,
		eTREASURE_PARFAIT_S = 18,
		eTREASURE_MEAT_L = 19,
		eTREASURE_MEAT_S = 20,
		eTREASURE_PIZZA = 21,
		eTREASURE_SFLAG = 22,
		eTREASURE_MAX = 23
	};

private:
	uint m_nKind{};

	uint m_nTreasure{};

	bool m_bBreak{};

	GameObject* m_viaAnimTarget{};

	int m_viaAnimCnt{};

	bool m_bBreakEnd{};

public:
	uint nKind();

	bool IsBreak();

	uint nTreasure();

	bool IsBreakEnd();

	cTaskBgBox();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool RegistData(const uint& kind, const uint& treasure, const Vector4& trans, const Vector4& rotate, const bool& isBreak = false);

	bool RegistData(sSaveDataBox& box);

	void HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW) override;
};

}
