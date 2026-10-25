#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemy : public cTaskChar
{
private:
	int m_nChangeMoveFrame{};

	int m_nRebirthTimer{};

	int m_nNoInputTimer{};

	int m_nLTimer{};

	short m_nFallPassDir{};

	Constants::FORMATION_INFO* m_FormInfo{};

	std::vector<Constants::FORM_GROUP_INFO*> m_pFormGroupInfo;

	Constants::FORM_GROUP_INFO* m_pFormNow{};

	Vector4 m_vFTarget{};

	float m_fDefMaxSpeed{};

	int m_nFormGroup{};

	int m_nFormNum{};

	int m_nFormPart{};

	int m_nHamariTimer{};

public:
	cTaskCharEnemy();

	cTaskCharEnemy(const string& key, uint option = 0u);

	cTaskCharEnemy(Object* data, uint option = 0u);

	virtual bool Create(uint no);

	short& nFallPassDir();

	int& nLTimer();

	int& nHamariTimer();

	int& nNoInputTimer();

	int& nRebirthTimer();

	int& nChangeMoveFrame();

	Constants::FORMATION_INFO*& pFormInfo();

	int& nFormPart();

	int& nFormGroup();

	int& nFormNum();

	Vector4& vFTarget();

	std::vector<Constants::FORM_GROUP_INFO*>& pFormGroupInfo();

	Constants::FORM_GROUP_INFO*& pFormNow();

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	void PresetEnv() override;

	void RegistPacket() override;

	void SyncPacket() override;

	static cTaskCharEnemy* Spawn(int kind, Vector4 pPos, float rad);

	static int Mms2Kind(int mmsKind);

	int Kind2ChipColor(int nKind);

	bool IsDistFallDir(ushort nDir, Vector4 pvSp, float fAccel);

	ushort nSearchFallDir(Vector4 pvSpeed, float fAccel);

	bool IsDistFallVec(Vector4 pvSp);

	bool IsDistBar(ushort nDir, Vector4 pvSp, float fAccel);

	cTaskCharEnemy* pDistBarChar(ushort nDir, Vector4 pvSp, float fAccel);

	ushort nReverseDir(ushort dir);

	ushort nSlantDir(ushort dir);

	ushort nRandomDir();

	ushort nTargetDir(float fDx, float fDz, float fDistAngle = 0.5f);

	ushort nTargetDir2(float fDx, float fDz, float fDistAngle = 0.5f);

	ushort nTargetDirMeet(float fDx, float fDz, float fDistAngle = 0.5f);

	ushort nFallPassDirReverse(ushort nDir, Vector4 pvSp, float fAccel);

	ushort nFallPassDirRegular(ushort nDir, Vector4 pvSp, float fAccel);

	ushort nFallPassDirRandom(ushort nDir, Vector4 pvSp, float fAccel);

	ushort nFallPassDirTarget(ushort nDir, Vector4 pvTarget, Vector4 pvSp, float fAccel);

	int FormNormal_Parent();

	int FormNormal_Child();

	int FormMeetSnake_Parent();

	int FormMoveSnake_Parent();

	int FormMeetCircle_Parent();

	int FormMoveCircle_Parent();

	int FormMeetArrow_Parent();

	int FormMoveArrow_Parent();

	int FormMeetFence_Parent();

	int FormMoveFence_Parent();

	int FormMeet_Children();

	int FormMove_Children();

	int Form_MMS2MeetFunc(int mmsType);

	int Form_MMS2MoveFunc(int mmsType);

	bool Form_MMS2MeetOrder(int mmsType);

	bool Formation_Parent(int nMes, cJetTask* pListC, cJetTask* pScene);

	bool Formation_Child(int nMes, cJetTask* pListC, cJetTask* pScene);

	cTaskCharPlayer* SearchNearPlayer();

	bool Status() override;

	bool TaikiAction() override;

	float GetHitPower(int nPart, int nOtherID) override;

	ushort nHamariRecoverDir(Vector4 vPos, Vector4 vTarget);
};

}
