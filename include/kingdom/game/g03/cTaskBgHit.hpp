#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgBase.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cFaceList.hpp"

namespace kingdom::game::g03
{

class cTaskBgHit : public cTaskBgBase
{
public:
	enum EState
	{
		STATE_NONE = 0,
		STATE_WOOD_BOX = 1,
		STATE_ELEVATOR = 2,
		STATE_SEESAW = 4,
		STATE_ROT_Y = 8,
		STATE_SLIDE = 0x10,
		STATE_FALL_FLOOR = 0x20,
		STATE_TRACK = 0x40,
		STATE_FALL_WALL = 0x80,
		STATE_MOVE_PATCH = 0x100,
		STATE_PISTON = 0x200
	};

private:
	uint m_nState{};

	cFaceList* m_pHitFace{};

	bool m_bMove{};

	bool m_bMoment{};

	float m_fMoment{};

	Matrix4x4 m_Mtx{};

	Matrix4x4 m_MtxInv{};

	Matrix4x4 m_MtxOld{};

	Matrix4x4 m_MtxInvOld{};

	Matrix4x4 m_MtxRotate{};

	Matrix4x4 m_MtxRotateInv{};

	Matrix4x4 m_MtxRltateOld{};

	Matrix4x4 m_MtxRltateInvOld{};

	Matrix4x4 m_MtxRotTrans{};

	bool m_bCollision{};

protected:
	ResNode* m_animRootNode{};

	bool m_isCalcedMtx{};

	bool m_forceUpdateMtx{};

public:
	cFaceList*& pFaceList();

	bool& IsMove();

	bool& IsMoment();

	float& fMoment();

	Matrix4x4& pMtx();

	Matrix4x4& pMtxInv();

	Matrix4x4& pMtxOld();

	Matrix4x4& pMtxInvOld();

	Matrix4x4& pMtxRotate();

	Matrix4x4& pMtxRotTrans();

	Matrix4x4& pMtxRotateInv();

	Matrix4x4& pMtxRotateOld();

	Matrix4x4& pMtxRotateInvOld();

	bool& IsCollision();

	uint& nState();

	void SetState(uint n);

	bool IsState(uint n);

	void OnState(uint n);

	void OffState(uint n);

	cTaskBgHit();

	cTaskBgHit(ResNode* const& node0);

	cTaskBgHit(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void UpdateMatrix();
	// the CHR slot whose "root" bone moves the collision (-1 = none), and the root fold of DOL 0x80154e64 (m_Mtx *= S * rt, m_MtxRotate *= r)
	int AnmRootSlot();
	void ApplyAnmRoot();

	void RegistHitData(string key);

	virtual void HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW);

	virtual void HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW);

protected:
	void ReleaseTerm() override { cTaskBgHit::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
