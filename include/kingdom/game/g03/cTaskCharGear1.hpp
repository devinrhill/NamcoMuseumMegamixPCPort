#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCharGear1 : public cTaskKingdom
{
public:
	static constexpr int MAX_LEVEL = 4;

	static constexpr uint GEAR_SMALL = 0u;

	static constexpr uint GEAR_LARGE = 1u;

	static constexpr int FLAG_REVERSE = 1;

	static constexpr int FLAG_STOP = 2;

private:
	uint m_nLevel{};

	uint m_nFlag{};

	uint m_nGearNum{};

	std::vector<uint> m_nGearKind;

	std::vector<float> m_nGearSpeedNow;

	float m_nGearSpeed[4]{};

public:
	void SetLevel(uint level);

	uint& nFlag();

	cTaskCharGear1(cTaskKingdom* pParent);

	virtual ~cTaskCharGear1();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	static bool s_CheckLimit(const Vector4& vPos);

	int GetGear(const Vector4& vPos);

	bool LimitLargeArea(Vector4& vOut, const Vector4& vPos, const Vector4& vVec);
};

}
