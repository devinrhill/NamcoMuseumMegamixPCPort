#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"

namespace kingdom::game::g07
{

class cTaskChar : public cTaskModel
{
public:
	enum EAnm
	{
		eANM_TYPE_NONE = 0,
		eANM_TYPE_CHRANM = 1,
		eANM_TYPE_CLRANM = 2
	};

	static constexpr int MO_REF_HAKI_TIME = 10;

	static constexpr int MO_REF_HIT_MAX = 10;

	static const MO_CHAR_PARAMS* s_DefParam;

private:
	Vector4 m_vMTrans{};

	Vector4 m_vMTransExec{};

	Vector4 m_vRotate{};

	Vector4 m_vMTransOld{};

	Vector4 m_vMBegin{};

	Matrix4x4 m_mMtx{};

	bool m_bMtxUsed{};

	cKingdomPtkHnd* m_phEffPiyo{};

	uint m_nGrobalID{};

	uint m_nID{};

	uint m_nKind{};

	uint m_nCharFlag{};

	uint m_nCharFlagOLD{};

	ushort m_nStep[4]{};

	ushort m_nMode{};

	ushort m_nOldMode{};

	ushort m_nNextMode{};

	ushort m_nReturnMode{};

	int m_nTimer{};

	bool m_bForceDeath{};

	MO_CHAR_PARAMS* m_Param{};

	MO_CHAR_PARAMS* m_pDefParam{};

	Vector4 m_vSpeed{};

	float m_fSpeed{};

	ushort m_nInputDir{};

	ushort m_nModelDir{};

	ushort m_nFallDir{};

	Vector4 m_vHSpeed{};

	uint m_nHereticTimerBase{};

	uint m_nHereticTimer{};

	int m_nLastRefID{};

	uint m_nLastRefCount{};

	uint m_nRefFlag{};

	int m_nRefChar{};

	int m_nRefCharOLD{};

	int m_nRefAxisOLD{};

	int m_nRefParts{};

	int m_nRefPartsOLD{};

	int m_nRefCounter{};

	int m_nRefHistIndex{};

	Constants::MO_CHAR_REFLECT* m_RefHistory[60]{};

	uint m_nHitFlag{};

	uint m_nHitFlagAll{};

	int m_nExHitNum{};

	std::vector<Constants::EX_HIT_PARAM*> m_pExHitParam;

	Constants::PANEL_INFO* m_pUGPanels[5]{};

	int m_nPiyoTimer{};

	Constants::CHAR_GIVE_POINTS* m_GivePoints{};

	short m_nModel{};

	ushort m_nEffectTimer{};

	int m_nChipColor{};

	int m_nMutekiTimer{};

public:
	bool CHECK_DELETE_REQ();

	cTaskChar();

	cTaskChar(const string& key, uint option = 0u);

	cTaskChar(Object* data, uint option = 0u);

	virtual uint GetPadNo();

	bool& bMtxUsed();

	bool& bForceDeath();

	Matrix4x4& pMatrix();

	Vector4& vMBegin();

	Vector4& vMBeginTrans();

	virtual bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene);

	uint GrobalID();

	void GrobalID(uint id);

	uint ID();

	void ID(uint id);

	uint& nKind();

	uint TestFlag(uint flag);

	bool bTestFlag(uint flag);

	void OffFlag(uint flag);

	void OnFlag(uint flag);

	void XchgFlag(uint flag);

	void FlagBackup();

	uint TestFlagOLD(uint flag);

	bool bTestFlagOLD(uint flag);

	uint TrigFlag(uint flag);

	bool bTrigFlag(uint flag);

	ushort& Step(int n = 0);

	int& nTimer();

	int GetMode();

	int SetMode(int mode);

	int OldMode();

	int NextMode();

	ushort ReturnMode();

	void ReturnMode(ushort n);

	int ForceMode(int mode);

	MO_CHAR_PARAMS*& pParam();

	MO_CHAR_PARAMS* pDefParam();

	float& fWeight();

	uint& nHereticVal();

	float& fMaxSpeed();

	float& fAccelSpeed();

	float& fDecelSpeed();

	float& fHitPower();

	float& fChargePower();

	float& fSlopePower();

	float& fWallReflect();

	float& fHitSize();

	float& fWHitSize();

	int& nPoints();

	int& nVSPoints();

	float& fExParam(int n);

	float fDefWeight();

	uint nDefHereticVal();

	float fDefMaxSpeed();

	float fDefAccelSpeed();

	float fDefDecelSpeed();

	float fDefHitPower();

	float fDefChargePower();

	float fDefSlopePower();

	float fDefWallReflect();

	float fDefHitSize();

	float fDefWHitSize();

	int nDefPoints();

	int nDefVSPoints();

	float fDefExParam(int n);

	Vector4& vSpeed();

	float& fSpeed();

	ushort& nInputDir();

	ushort& nModelDir();

	ushort& nFallDir();

	Vector4& vHSpeed();

	uint& nHereticTimerBase();

	uint& nHereticTimer();

	float HereticRate();

	void SetHeresy(uint time);

	int& nLastRefID();

	uint& nLastRefCount();

	uint& RefFlag();

	bool bRefFlag();

	int& RefChar();

	int& RefCharOLD();

	int& RefAxisOLD();

	int& RefParts();

	bool bRefParts();

	int& RefPartsOLD();

	bool bRefPartsOLD();

	int& RefCounter();

	Constants::MO_CHAR_REFLECT* RefHistory(int n);

	virtual void RefAttacked(int nParts, int nGrobalID);

	virtual void RefAttack(int nParts, int nGrobalID);

	uint& nHitFlag();

	bool bnHitFlag();

	uint& nHitFlagAll();

	int nExHitNum();

	bool bnExHitNum();

	Constants::EX_HIT_PARAM* ExHitParam(int n);

	Constants::PANEL_INFO* pUGPanel(int n = 0);

	virtual float GetHitPower(int nPart, int nOtherID);

	virtual float GetChargePower(int nPart, int nOtherID);

	Constants::CHAR_GIVE_POINTS*& pGivePoints();

	int& nPiyoTimer();

	short& nModel();

	ushort& nEffectTimer();

	virtual void RegistPacket();

	virtual void SyncPacket();

	int& nChipColor();

	virtual void FallEffect(int nRefID);

	int& nMutekiTimer();

	virtual bool IsFall(float x, float y, float z);

	virtual bool TaikiAction();

	virtual uint Input();

	virtual void SetMMSParam(int para1, int para2, int para3);

	virtual void DeleteReq();

	virtual void SetFrame(uint model, float frame);

	virtual float GetFrame(uint model);

	virtual bool AnimateModel(uint model);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void UpdateFrame() override;

	Vector4& vWTrans();

	Vector4& vMTrans();

	Vector4& vMTransOLD();

	Vector4& vRotate();

	Vector4& vScale();

	virtual void UpdateMatrix();

	void RollMatrix(Matrix4x4& _out, int model, const Vector4& speed, const string& nodename);

	void RollMatrix(Matrix4x4& _out, int model, const string& nodename);

	void SetDrawRollMatrix(Vector4 trans, Matrix4x4& roll, int model);

	void SetDrawRollMatrix(Matrix4x4& roll, int model);

	virtual bool Move();

	virtual bool Status();

	virtual bool ActionBoth();

	virtual bool ActionLocal();

	virtual bool ActionOther();

	virtual void NormalMove();

	virtual bool FallMove();

	virtual void ChangeFallMove();

	virtual void ChangeTaiki();

	virtual void ExecGivePoints(int id, int add_point = -1, int sub_point = -1);

	virtual bool MapMove(Vector4& pOutTrans, Vector4& pOutVec, Vector4& pOutSpeed, Vector4 pVec);

	virtual bool MapMove(Vector4& pOut, Vector4 pVec);

	virtual void PresetEnv();

	virtual void InitParam(MO_CHAR_PARAMS* pParam);

	int UpdateUGPanel(Nullable<Vector4> pVec);

	virtual bool IsFall(Nullable<Vector4> pVec = nullptr);

	virtual bool CheckFall(Vector4 pVec);

	virtual void AdjustMaxSpeed(Vector4& pVec);

	virtual void AdjustMaxSpeed();

	virtual void DecelSpeed(int input_dir);

	void RegistRefHistory(int id, int count, uint flag = 0u, int otherID = 0, float pow = 0.0f, float charge = 0.0f);

	Constants::MO_CHAR_REFLECT* SearchRefHistory(int id);

	void UpdateRefHistory();

	Constants::MO_CHAR_REFLECT* RefHistoryOffset(int n);

	virtual void ExecReflect(NET_RF_PACKET* pPacket);

	virtual void ExecReflectSub(Vector4 pVec, float hit_pow, float charge_pow);

	int SendMessage(int nMes, cJetTask* pListC = nullptr, cJetTask* pScene = nullptr);

	int ExecModeFunc(cJetTask* pListC = nullptr, cJetTask* pScene = nullptr);

	virtual bool AdjustSpeedByHitAll(Vector4& pOutSpeed, Vector4 pTrans, Vector4 pSpeed);

	virtual bool AdjustSpeedByHit(cTaskChar* pChar, Vector4& pOutSpeed, Vector4 pTrans, Vector4 pSpeed);

	virtual bool AdjustSpeedByHit(cTaskChar* pChar);

	virtual void Dir2Rotate(int dir);

	virtual void Rotate2Dir(float rot);

	virtual bool ExHitCrossPoint(Vector4& pOut, Vector4 pPos, Vector4 pSpeed, float HitSize);

	Constants::EX_HIT_PARAM* ExHitRegistRect(int n, Vector4 pOrg, float width, float depth, int height = 1);

	Constants::EX_HIT_PARAM* ExHitRegistCircle(int n, Vector4 pOrg, float range, int height = 1);

	void ExHitAlloc(int num);

	virtual void GetItem(int nGrobalID);

	virtual void CalcSpeed(Vector4& pSpeed);

	void SetMaterialAlpha(uint n, byte alpha);

	virtual bool IsModelAnmReverse(uint nModel, uint nSlot, uint type);

	virtual void MortionReverse(uint nModel, uint nSlot, uint type);

	virtual void SetMortion(uint nModel, string name, uint nSlot = 0u, uint type = 0u);

	virtual void SetMortionReverse(uint nModel, string name, uint nSlot, uint type = 0u);

protected:
	void ReleaseTerm() override { cTaskChar::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
