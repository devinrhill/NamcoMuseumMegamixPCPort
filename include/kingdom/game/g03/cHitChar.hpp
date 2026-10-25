#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{
class cTaskChar;
}

namespace kingdom::game::g03
{

class cHitChar
{
public:
	enum EFlag
	{
		FLAG_ACTIVE = 1,
		FLAG_ATTACK = 2,
		FLAG_DEFENSE = 4,
		FLAG_PUSHOUT = 8,
		FLAG_ANTIKNIGHT = 16,
		FLAG_BOUNDCENTER = 32,
		FLAG_END = 33
	};

	enum ESide
	{
		SIDE_PLAYER = 0,
		SIDE_ENEMY = 1
	};

private:
	static constexpr float G03_DEFAULT_RANGE = 0.38f;

	cTaskChar* m_pParent{};

	uint m_nSide{};

	uint m_nFlag{};

	Vector4 m_vOfsPos{};

	float m_fOfsRange{};

	Vector4 m_vBoundCenter{};

	Vector4 m_vPos{};

	float m_fRange{};

public:
	virtual void SetOfsPos(Vector4 vPos);

	virtual void SetOfsPos(float x, float y, float z);

	virtual void SetBoundCenter(Vector4 vPos);

	virtual void SetBoundCenter(float x, float y, float z);

	virtual void SetRange(float fRange);

	uint FlagTest(uint nFlag);

	bool bFlagTest(uint nFlag);

	bool FlagAccod(uint nFlag);

	void FlagOn(uint nFlag);

	void FlagOff(uint nFlag);

	uint nSide();

	cTaskChar* pParent();

	Vector4 vOfsPos();

	float fOfsRange();

	Vector4 vPos();

	Vector4 vBoundCenter();

	float fRange2();

	cHitChar(cTaskChar* pParent, int nSide, int nFlag = 15);

	virtual ~cHitChar();

	virtual void UpdateMatrix(const Matrix4x4& m44);
};

}
