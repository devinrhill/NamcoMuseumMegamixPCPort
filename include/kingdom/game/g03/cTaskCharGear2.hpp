#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{
class cTaskCharGolvis05;
}

namespace kingdom::game::g03
{

class cTaskCharGear2 : public cTaskKingdom
{
public:
	enum EGearsHigh
	{
		GEARS_HIGH_0 = 0,
		GEARS_HIGH_1 = 1,
		GEARS_HIGH_2 = 2
	};

	enum EGearMode
	{
		GEAR_MODE_UP = 0,
		GEAR_MODE_DOWN = 1
	};

	enum EGearStep
	{
		GEAR_STEP_NORMAL = 0,
		GEAR_STEP_CHANGE = 1,
		GEAR_STEP_STOP = 2
	};

	enum EGearRotate
	{
		GEAR_ROTATE_NONE = 0,
		GEAR_ROTATE_LEFT = 1,
		GEAR_ROTATE_RIGHT = 2
	};

private:
	class GEARS_INFO
	{
	public:
		uint m_nKind{};

		uint m_nMode{};

		float m_nSpeedNow{};

		int m_nHeight{};

		int m_nStep{};

		int m_nRotateStep{};

		int m_nRotateDir{};
	};

public:
	static constexpr int MAX_LEVEL = 4;

	static constexpr int GEAR_LARGE_MAX = 1;

	static constexpr int GEAR_SMALE_MAX = 4;

	static constexpr int GEAE_NUM_MAX = 5;

	static constexpr int GEAR_SMALL = 0;

	static constexpr int GEAR_LARGE = 1;

	static constexpr int FLAG_STOP = 1;

	static constexpr int FLAG_PAUSE = 1;

private:
	uint m_nLevel{};

	uint m_nFlag{};

	uint m_nGearNum{};

	cTaskCharGolvis05* m_pBoss{};

	std::vector<cTaskCharGear2::GEARS_INFO*> m_pInfo;

	float m_nGearSpeed[4]{};

public:
	static const float m_fGearsForceDirs[4];

private:
	static const Vector4 vPosList[4];

public:
	uint& nFlag();

	void SetLevel(uint level);

	cTaskCharGolvis05* pBoss();

	cTaskCharGear2(cTaskKingdom* pParent);

	virtual ~cTaskCharGear2();

	bool Execute(cJetTask* pParam) override;

	void RotateGear(uint nGear);

	void RiviseRotateVector(Vector4& v);

	void Draw(int nStatus, cJetTask* pParam) override;

	void ChangeHeight(int nIndex, int nLevel);



	bool LimitLargeArea(Vector4& vOut, const Vector4& vPos, const Vector4& vVec);

	uint GetRotateGearNo(const Vector4& v);
};

}
