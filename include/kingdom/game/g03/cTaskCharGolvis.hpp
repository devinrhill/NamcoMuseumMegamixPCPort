#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharEnemy.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "kingdom/game/g03/cTaskCharBEye.hpp"
#include "kingdom/game/g03/cTaskLandAttack.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis : public cTaskCharEnemy
{
public:
	enum EMdlKind
	{
		MDL_KIND_NORMAL = 0,
		MDL_KIND_NORMALB = 1,
		MDL_KIND_BIJIKE = 2,
		MDL_KIND_BIJIKEB = 3,
		MDL_KIND_EYE = 4,
		MDL_KIND_MAX = 5
	};

	enum EAnmKind
	{
		ANM_KIND_WAIT = 0,
		ANM_KIND_MOVE = 1,
		ANM_KIND_ESCAPE = 2,
		ANM_KIND_APPEAR = 3,
		ANM_KIND_REBIRTH = 4,
		ANM_KIND_RECOVER = 5,
		ANM_KIND_LAUGH = 6,
		ANM_KIND_DAMAGE = 7,
		ANM_KIND_SUFFER = 8,
		ANM_KIND_PLAY00 = 9,
		ANM_KIND_PLAY01 = 10,
		ANM_KIND_PLAY02 = 11,
		ANM_KIND_INSTRUCT = 12,
		ANM_KIND_KNOCK_H0 = 13,
		ANM_KIND_KNOCK_V0 = 14,
		ANM_KIND_KNOCK_H1 = 15,
		ANM_KIND_KNOCK_V1 = 16,
		ANM_KIND_HEADBUTT0 = 17,
		ANM_KIND_HEADBUTT1 = 18,
		ANM_KIND_HEADBUTT2L = 19,
		ANM_KIND_HEADBUTT3 = 20,
		ANM_KIND_FAN_START = 21,
		ANM_KIND_FAN = 22,
		ANM_KIND_FAN_END = 23,
		ANM_KIND_DIZZ = 24,
		ANM_KIND_DOWN = 25,
		ANM_KIND_DOWNNING = 26,
		ANM_KIND_WAKEUP = 27,
		ANM_KIND_BOMB_KNOCK = 28,
		ANM_KIND_BOMB_DRIBBLE = 29,
		ANM_KIND_BIJIKE_MOVE = 30,
		ANM_KIND_BIJIKE_EATEN = 31,
		ANM_KIND_EYE_MOVE = 32,
		ANM_KIND_DEMO_ESCAPE = 33,
		ANM_KIND_MAX = 34
	};

	class sBossDeathDemo
	{
	public:
		uint nProc{};

		bool bChange{};

		bool bUseDeathDemo{};

		sBossDeathDemo();
	};

private:
	class GOLVIS_NAME2ID
	{
	public:
		int m_nID{};

		string m_pName{};

		int m_num{};
	};

public:
	static constexpr float GOLBYS_GRAVITY_ACCEL = 1.0f;

protected:
	uint m_nAnmNew{};

	uint m_nAnmNow{};

	uint m_nAnmOld{};

	uint m_nAnmSw{};

	uint m_nAnmChgCount{};

	uint m_nAnmChgTime{};

	cTaskCharGolvis::sBossDeathDemo* m_sDeathDemo{};

	float m_fWindGuiterLength{};

	bool m_bLive{};

	bool m_bDisp{};

	bool m_bDeathAnime{};

	bool m_bEnableGravity{};

	bool m_bOnFloor{};

	Constants::gmSGolvisAttack* m_pAttackTBL[72]{};

	float m_fBombRange{};

	int m_iBombInterval{};

	int m_iBombAimTime{};

	float m_fBombSightMoveSpeed{};

	float m_fMoveSpeed{};

	float m_fBijikeMoveSpeed{};

	float m_fMoveSpeedNow{};

	float m_fHitRange{};

	float m_fBijikeHitRange{};

	float m_fFanPower{};

	int m_iCount[3]{};

	float m_fCount[2]{};

	Vector4 m_vPos[3]{};

	int m_iBlinkCount{};

	int m_iDispColor{};

	int m_iIzikeLevel{};

	int m_iEatenNum{};

	int m_iIzikeCount{};

	Vector4 m_vFirstTrans{};

	Vector4 m_vFirstRotate{};

	float m_fRadius{};

	float m_fRadiusHit{};

	float m_fHeight{};

	float m_fLowHeight{};

	int m_iAIMode{};

	int m_iSubState{};

	int m_iAttackTime[8]{};

	bool m_bDispBall{};

	int m_iEffectTimer{};

	bool m_bExistPiyoriEffect{};

	bool m_bExistCycloneSE{};

	int m_iCycloneWindSECount{};

	bool m_bExistGuitarSE{};

	int m_iMotionChangeTimer{};

	uint m_uiPacLife{};

	bool m_bPacDamage{};

	bool m_bLaughEnable{};

	cTaskLandAttack* m_pAttackLand{};

	Matrix4x4 m_PtkConfMtx{};

	cKingdomPtkHnd* m_hPtkStorm{};

	cKingdomPtkHnd* m_hPtkStorm2{};

	cKingdomPtkHnd* m_hPtkConf{};

	cKingdomPtkHnd* m_hPtkSweat{};

public:
	cTaskCharBEye* m_pBijikeEyeObj{};

protected:
	bool m_bIjikeMsgCome{};

	bool m_bGhostCreateFlag{};

	bool m_bAttackFlag1{};

	bool m_bAttackFlag2{};

	GameObject* m_hSndPiyori{};


	static constexpr float MOVE_SPEED_DELTA = 0.0009f;

	static constexpr float CHAR_R_NORMAL = 1.4f;

	static constexpr float CHAR_R_HIT_NORMAL = 1.45f;

	static constexpr float CHAR_HEIGHT_NORMAL = 5.0f;

	static constexpr float CHAR_LOWHEIGHT_NORMAL = 0.0f;

	static constexpr float CHAR_R_IZIKE = 0.55f;

	static constexpr float CHAR_R_HIT_IZIKE = 0.6f;

	static constexpr float CHAR_HEIGHT_IZIKE = 2.0f;

	static constexpr float CHAR_LOWHEIGHT_IZIKE = 0.0f;

	static constexpr float REVIVE_MIN_SCALE = 0.45f;

	static constexpr float REVIVE_SCALING_START = 28.0f;

	static constexpr float REVIVE_SCALING_END = 40.0f;

	static constexpr float APPEAR_VOICE_FRAME = 36.0f;

private:
	static const cTaskCharGolvis::GOLVIS_NAME2ID* s_aAttackName[59];

public:
	virtual Vector4 CalcButtCursorTrans(Vector4 v);

	bool& bUseDeathDemo();

	uint GetDeathProc();

	void ChangeDeathProc(uint nProc);

	bool IsDeathProcFirstCall();

protected:
	virtual void MonsterFrameMove_PreProcess();

	virtual void MonsterFrameMove_Free();

	virtual void MonsterFrameMove_Escape();

	virtual bool MonsterFrameMove_Death();

	virtual bool IsForceAttack();

	virtual cTaskLandAttack* pAttackLand();

public:
	virtual void InitStateFirst();

	virtual Vector4 vFirstTrans();

	virtual Vector4 vFirstRotate();

	virtual void InitStateStart();

	virtual Constants::gmSGolvisAttack* GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum);

	virtual bool IsBossDeath();

	bool IsBijikeMsgCome();

	void IjikeStart() override;

	uint nLife();

	int GetBijikeLevel();

	int GetSubState();

	int GetAIMode();

	int GetCount(uint val);

	cTaskCharGolvis(cJetTask* pParent);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void SetConfEffect();

	void EraseConfEffect();

	void SetModel() override;

	virtual cTaskModel::cPBModel* pCorrectModel();

	virtual void SetAnmState(const uint& state);

	virtual void SetAnmState(const uint& state, const uint& nTime);

	uint GetAnmState() override;

	virtual void SetAnm(string anime, uint idx = 0u);

	virtual float GetAnmRate(uint idx);

	virtual void AnmExecBlend();

	virtual bool IsAnmEnd();

	static cTaskCharGolvis* Create(cJetTask* pParent, uint nRound);

	virtual void Setting(bool bRelocate);

protected:
	virtual void ChangeToFree();

	virtual void ChangeToEscape();

	virtual void ChangeToDamage();

	virtual void ChangeToPossess();

	virtual void ChangeToKnock();

	virtual bool ChangeToAttack(int iAttackKind);

	virtual void ChangeToMotion(uint uiMotGID);

	virtual void ChangeToAppear();

	IEnumerator* WaitOneFramePlaySE();

public:
	virtual void ChangeToRevive();

protected:
	virtual void ChangeToMedama();

	virtual bool TryChangeToAttack();

	virtual bool TryChangeToLaugh();

	virtual void AttackProcess();

	virtual void MonsterFrameMove();

	virtual void MonsterRender();

public:
	virtual int Attack(Vector4 pvLocalAttackPos, float fAttackR, Vector4 pvFlipCenterPos, float fAttackSpeed, bool bKabutoGuardEnable);

	void RequestEffectAround(int iKind, int iInterval = 0);

	virtual bool HitCheckPacman();

	virtual void FallProcess();

	virtual bool IsOnFloor();

	virtual bool GetFloorHeight(float& pOut, Vector4 vPos = Vector4());

	virtual bool GetFloor(Vector4& pOut, Vector4 vPos = Vector4());

protected:
	virtual void AttackLand(int nKind);

	virtual void AttackLandRegist(cTaskBgGimmick* pGimmick);

	virtual bool AttackLandIsEnd();

	virtual void AttackLandPause(bool nFlag = true);

	virtual bool AttackLandGetMsg(uint& pMes);

	virtual uint AttackName2ID(string pName);

protected:
	void ReleaseTerm() override { cTaskCharGolvis::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
