#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

class cTaskCharItem : public cTaskCharEnemy
{
public:
	float m_fWeight{};

	uint m_nHereticVal{};

	float m_fMaxSpeed{};

	float m_fAccelSpeed{};

	float m_fDecelSpeed{};

	float m_fHitPower{};

	float m_fChargePower{};

	float m_fSlopePower{};

	float m_fWallReflect{};

	float m_fHitSize{};

	float m_fWHitSize{};

	int m_nPoints{};

	int m_nVSPoints{};

	float m_fExParam[3]{};

private:
	static const MO_CHAR_PARAMS* g_MoItemParam[4];

public:
	cTaskCharItem();

	bool Create(uint no) override;

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	virtual void Setting(int kind);

protected:
	void ReleaseTerm() override { cTaskCharItem::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
