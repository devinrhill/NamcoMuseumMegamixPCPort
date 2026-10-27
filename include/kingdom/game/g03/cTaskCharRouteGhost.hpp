#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cHitChar.hpp"

namespace kingdom::game::g03
{

class cTaskCharRouteGhost : public cTaskCharGhost
{
public:
	enum EType
	{
		TYPE_NORMAL = 0,
		TYPE_IZIKE = 1,
		TYPE_RIDE = 2,
		TYPE_MAX = 3
	};

	enum EState
	{
		STATE_NORMAL = 0,
		STATE_IZIKE = 1,
		STATE_MAX = 2
	};

	enum EGhostST
	{
		GHOST_ST_NONE = 0,
		GHOST_ST_MOVEING = 1,
		GHOST_ST_ATTACK = 2,
		GHOST_ST_IZIKE_PRE = 3,
		GHOST_ST_IZIKE = 4,
		GHOST_ST_EATED = 5,
		GHOST_ST_DEAD = 6,
		GHOST_ST_CREATE = 7,
		GHOST_ST_CREATE_MOVE = 8,
		GHOST_ST_END = 9
	};

	enum ESNDType
	{
		SND_TYPE_NONE = 0,
		SND_TYPE_ENGGINE1 = 1,
		SND_TYPE_ENGGINE2 = 2
	};

private:
	uint m_nGhostRider{};

	int m_nCount{};

	bool m_bMove{};

	float m_fSpeed{};

	float m_fRouteMaxSpeed{};

	float m_fTargetSpeed{};

	int m_nTargetPointID{};

	Vector4 m_vMoveTarget{};

	float m_fEatSpeed{};

	int m_nBombStatus{};

	int m_nBombCounter{};

	float m_fMaxSpeed{};

	float m_fAddSpeed{};

	float m_fTurnSpeed{};

	float m_fIzikeSpeed{};

	int m_nIzikeFlashTime{};

	uint m_nSndType{};

	GameObject* m_SndHndEngine{};

	bool m_bPackAttack{};

	float m_fFootDist{};

	Vector4 m_vFootPos{};

	cKingdomPtkHnd* m_PtkHndEffect[2]{};

	Vector4 m_vEffectNode[2]{};

	string GetGhostModelKey();

	string GetGhostAnmKey();

	string GetGhostSRTAnmKey();

public:
	void SetTargetSpeed(int percent);

	uint& nGhostRider();

	int& nTargetPointID();

	Vector4& vMoveTarget();

	Vector4& vFootPos();

	Vector4& GetBomsShootPos();

	uint& nSndType();

	cKingdomPtkHnd* pPtkHndEffect(int num);

	uint GetSnaipeTime();

	virtual cTaskChar::eKIND GetKind();

	virtual string GetModelKey();

	virtual string GetAnmKey();

	virtual string GetNodeKey0();

	virtual string GetNodeKey1();

	virtual uint GetEffectType0();

	virtual uint GetEffectType1();

	bool IsDead() override;

	void SetDrawShadow(bool draw);

	cTaskCharRouteGhost();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void SetType(const uint& type);

	void SetAnmState(const uint& state);

	void SetAnm(string anime, uint idx = 0u);

	void SetModel() override;

	void SetParam(int kind);

	void SetEffect();

	void SetOptionItem(int type);

	void SetBombType(int type);

	void FastDead();

private:
	void CheckStatus();

public:
	void Move();

	void Attacked(cHitChar* pRev, cHitChar* pAtk) override;

	void HitPushOut(Vector4 vVec, cTaskChar* pChar) override;

protected:
	void ReleaseTerm() override { cTaskCharRouteGhost::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
