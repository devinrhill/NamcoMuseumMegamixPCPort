#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g03/cHitChar.hpp"

namespace kingdom::game::g03
{

class cTaskChar : public cTaskModel
{
public:
	enum eKIND
	{
		eKIND_PACMAN = 0,
		eKIND_CLYDE = 1,
		eKIND_PINKY = 2,
		eKIND_BLINKY = 3,
		eKIND_INKY = 4,
		eKIND_BOSS = 5,
		eKIND_BOSS_IJIKE = 6,
		eKIND_BOSS_EYE = 7,
		eKIND_RIDE_CAR = 8,
		eKIND_RIDE_UFO = 9,
		eKIND_RIDE_BOAT = 10,
		eKIND_RIDE_PLANE = 11,
		eKIND_RIDE_HOVER = 12,
		eKIND_MAX = 13
	};

private:
	cTaskChar::eKIND m_nKind{};

	uint m_nType{};

	uint m_nState{};

	uint m_nStateOld{};

	float m_fRadius{};

	uint m_nPushOutLevel{};

	Vector4 m_vTrans{};

	Vector4 m_vTransOld{};

	Vector4 m_vRotate{};

	Vector4 m_vScale{};

	Vector4 m_vMove{};

	Vector4 m_vAccel{};

	std::vector<cHitChar*> m_vecHitChar;

public:
	cTaskChar::eKIND& nKind();

	uint& nType();

	uint& nState();

	void SetState(uint n);

	bool IsState(uint n);

	void OnState(uint n);

	void OffState(uint n);

	uint& nStateOld();

	void SetStateOld(uint n);

	bool IsStateOld(uint n);

	float& fRadius();

	uint& nPushOutLevel();

	virtual uint GetAnmState();

	Vector4& vTrans();

	float vTrans(int n);

	Vector4& vTransOld();

	float vTransOld(int n);

	Vector4& vMove();

	float vMove(int n);

	Vector4& vAccel();

	float vAccel(int n);

	Vector4& vRotate();

	float vRotate(int n);

	Vector4& vScale();

	float vScale(int n);

	cTaskModel::cPBModel* pCurrentModel();

	std::vector<cHitChar*>& pvecHitChar();

	cHitChar* pHitChar(int n);

	virtual void HitPushOut(Vector4 vVec, cTaskChar* pChar);

	virtual void IjikeStart();

	virtual void IjikeEnd();

	cTaskChar();

private:
	void Init();

public:
	void Term() override;

protected:
	bool CheckRepetition(AnimationClip* data, uint idx);

public:
	void SetMaterialAlpha(uint n, byte alpha);

	virtual void Attacked(cHitChar* pRev, cHitChar* pAtk);

	void HitUpdate();

	void HitListup(std::vector<cHitChar*>& pVec, const int& nSide, const uint& nFlag);
};

}
