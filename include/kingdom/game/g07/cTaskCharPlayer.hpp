#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/cTaskCharHoipoi.hpp"

namespace kingdom::game::g07
{

class cTaskCharPlayer : public cTaskChar
{
public:
	class sPLAYER_LIST
	{
	public:
		string Player[4]{};
	};

	class sANIMATION_LIST
	{
	public:
		string anm[5]{};
	};

private:
	enum EKey
	{
		eUP = 0,
		eDOWN = 1,
		eLEFT = 2,
		eRIGHT = 3,
		eALL = 4
	};

public:
	static constexpr float PLAYER_DEFAULT_HEIGHT = 0.5f;

	static constexpr int SP_ATTACK_TABOO_TIME = 30;

	static constexpr float PLAYER_REVERSE_HEIGHT = 2.25f;

	static constexpr float PLAYER_REVERSE_FALL_SPEED = 0.15f;

	static constexpr int REC_MAX = 6;

private:
	static const cTaskCharPlayer::sANIMATION_LIST* AnimationName[4];

	uint m_prevAnimNo = 999u;

public:
	cTaskCharHoipoi* m_pHoipoi{};

private:
	cKingdomPtkHnd* m_hEffCharge{};

	Constants::sPLAYER_CURSOR_PTK* m_sPlayerPtk{};

	Matrix4x4 m_mPtkPlayerNo{};

	Constants::MO_PLAYER_PARAMS* m_PLParam{};

	uint m_nPlayerNo{};

	int m_nPowerLevel{};

	int m_nHoverLevel{};

	int m_nChargeLevel{};

	int m_nChargePowLevel{};

	int m_nChargePowLevelOLD{};

	int m_nChargeCnt{};

	int m_nChargeWait{};

	uint m_nHoverTimer{};

	int m_nFunbariMeter{};

	int m_nFunbariHeretic{};

	int m_nSPCount{};

	int m_nArrowTimer{};

	float m_fReverseHeight{};

	uint m_nHoverWait{};

	int m_nConsState{};

	bool m_bHoiPoiRelease{};

	int m_nChargeInterval{};

	Vector4 m_vInputRec[6]{};

	Vector4 m_vInputRecEx[6]{};

	uint m_nRecoordPos{};

	bool m_bWiiSwing{};

	bool m_bWiiSwingEx{};

	uint m_nSpTimer{};

	float m_fReverseHeigthRate{};

	bool m_bPlayerMoveStop{};

public:
	cTaskCharPlayer();

	cTaskCharPlayer(const string& key, uint option = 0u);

	cTaskCharPlayer(Object* data, uint option = 0u);

	bool& bPlayerMoveStop();

	Constants::MO_PLAYER_PARAMS*& pPLParam();

	float& fMaxSpeedHover();

	float& fAccelSpeedHover();

	float& fDecelSpeedHover();

	uint& nHoverTimer();

	int& nPowerLevel();

	int& nHoverLevel();

	int& nChargeLevel();

	int& nChargePowLevel();

	int& nChargePowLevelOLD();

	int& nChargeCnt();

	int& nFunbariMeter();

	int& nFunbariHeretic();

	int& nSPCount();

	bool IsFall(float x, float y, float z) override;

	void ConsolValState(int state);

	int ConsolValState();

	int& nChargeInterval();

	void SetFrame(uint model, float frame) override;

	float GetFrame(uint model) override;

private:
	void Init();

public:
	void Term() override;

	void UpdateMatrix() override;

	bool Create(uint no);

	virtual void SetAnimation(uint AnimNo);

	uint WiiPadInput(const Vector4& rec, const Vector4& rec2);

	uint GetPadNo() override;

	void InitRecoord();

	uint Input() override;

	bool Execute(cJetTask* pParam) override;

	void PresetEnv() override;

	void ChargeSoundStop();

	bool Status() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	bool TaikiAction() override;

	void ExecGivePoints(int id, int add_point = -1, int sub_point = -1) override;

	int ChargeCnt2Level(int n);

	void InitParam(MO_CHAR_PARAMS* pParam) override;

	void AdjustMaxSpeed() override;

	void DecelSpeed(int input_dir) override;

	void RegistPacket() override;

	void SyncPacket() override;

	float ConsolValPower();

	float ConsolValHover();

	float ConsolValCharge();

	virtual void UseItem();

	bool ExecRebirth();

	bool ChangeFunbari();

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	bool IsFall(Nullable<Vector4> pVec = nullptr) override;

	bool RegistPreset();

	void CreateDashEffect();

	void RefAttacked(int nParts, int nGrobalID) override;

	float GetHitPower(int nPart, int nOtherID) override;

private:
	bool GetPadCnt(int padNum, cTaskCharPlayer::EKey key);

	bool GetPadTrg(int padNum, cTaskCharPlayer::EKey key);

	bool GetPadCntOrTrg(int padNum, cTaskCharPlayer::EKey key);

	bool GetPadVSRebirth(int padNum);

	int GetPadDir(int padNum);

	Vector2 GetPadLStickOrKey(int padNum);
};

}
