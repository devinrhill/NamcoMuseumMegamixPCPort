#include "kingdom/game/g07/cTaskChar.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <limits>
#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/MAP_HIT_RESULT.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

const MO_CHAR_PARAMS* cTaskChar::s_DefParam = new MO_CHAR_PARAMS();

bool cTaskChar::CHECK_DELETE_REQ()
{
	if (bTestFlag(8388608u))
	{
		return true;
	}
	return false;
}

cTaskChar::cTaskChar()
{
	Init();
}

cTaskChar::cTaskChar(const string& key, uint option)
	: cTaskModel(key, option)
{
	Init();
}

cTaskChar::cTaskChar(Object* data, uint option)
	: cTaskModel(data, option)
{
	Init();
}

uint cTaskChar::GetPadNo()
{
	CustomKingdomUtil::Assert("null");
	return 0u;
}

bool& cTaskChar::bMtxUsed()
{
	return m_bMtxUsed;
}

bool& cTaskChar::bForceDeath()
{
	return m_bForceDeath;
}

Matrix4x4& cTaskChar::pMatrix()
{
	return m_mMtx;
}

Vector4& cTaskChar::vMBegin()
{
	return m_vMBegin;
}

Vector4& cTaskChar::vMBeginTrans()
{
	return m_vMBegin;
}

bool cTaskChar::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	return true;
}

uint cTaskChar::GrobalID()
{
	return m_nGrobalID;
}

void cTaskChar::GrobalID(uint id)
{
	m_nGrobalID = id;
}

uint cTaskChar::ID()
{
	return m_nID;
}

void cTaskChar::ID(uint id)
{
	m_nID = id;
}

uint& cTaskChar::nKind()
{
	return m_nKind;
}

uint cTaskChar::TestFlag(uint flag)
{
	return m_nCharFlag & flag;
}

bool cTaskChar::bTestFlag(uint flag)
{
	return (m_nCharFlag & flag) != 0;
}

void cTaskChar::OffFlag(uint flag)
{
	m_nCharFlag &= ~flag;
}

void cTaskChar::OnFlag(uint flag)
{
	m_nCharFlag |= flag;
}

void cTaskChar::XchgFlag(uint flag)
{
	m_nCharFlag ^= flag;
}

void cTaskChar::FlagBackup()
{
	m_nCharFlagOLD = m_nCharFlag;
}

uint cTaskChar::TestFlagOLD(uint flag)
{
	return m_nCharFlagOLD & flag;
}

bool cTaskChar::bTestFlagOLD(uint flag)
{
	return (m_nCharFlagOLD & flag) != 0;
}

uint cTaskChar::TrigFlag(uint flag)
{
	return TestFlag(flag) & ~TestFlagOLD(flag);
}

bool cTaskChar::bTrigFlag(uint flag)
{
	return (TestFlag(flag) & ~TestFlagOLD(flag)) != 0;
}

ushort& cTaskChar::Step(int n)
{
	return m_nStep[n];
}

int& cTaskChar::nTimer()
{
	return m_nTimer;
}

int cTaskChar::GetMode()
{
	return m_nMode;
}

int cTaskChar::SetMode(int mode)
{
	m_nNextMode = (ushort)mode;
	return GetMode();
}

int cTaskChar::OldMode()
{
	return m_nOldMode;
}

int cTaskChar::NextMode()
{
	return m_nNextMode;
}

ushort cTaskChar::ReturnMode()
{
	return m_nReturnMode;
}

void cTaskChar::ReturnMode(ushort n)
{
	m_nReturnMode = n;
}

int cTaskChar::ForceMode(int mode)
{
	m_nNextMode = (m_nMode = (ushort)mode);
	return GetMode();
}

MO_CHAR_PARAMS*& cTaskChar::pParam()
{
	return m_Param;
}

MO_CHAR_PARAMS* cTaskChar::pDefParam()
{
	return m_pDefParam;
}

float& cTaskChar::fWeight()
{
	return m_Param->m_fWeight;
}

uint& cTaskChar::nHereticVal()
{
	return m_Param->m_nHereticVal;
}

float& cTaskChar::fMaxSpeed()
{
	return m_Param->m_fMaxSpeed;
}

float& cTaskChar::fAccelSpeed()
{
	return m_Param->m_fAccelSpeed;
}

float& cTaskChar::fDecelSpeed()
{
	return m_Param->m_fDecelSpeed;
}

float& cTaskChar::fHitPower()
{
	return m_Param->m_fHitPower;
}

float& cTaskChar::fChargePower()
{
	return m_Param->m_fChargePower;
}

float& cTaskChar::fSlopePower()
{
	return m_Param->m_fSlopePower;
}

float& cTaskChar::fWallReflect()
{
	return m_Param->m_fWallReflect;
}

float& cTaskChar::fHitSize()
{
	return m_Param->m_fHitSize;
}

float& cTaskChar::fWHitSize()
{
	return m_Param->m_fWHitSize;
}

int& cTaskChar::nPoints()
{
	return m_Param->m_nPoints;
}

int& cTaskChar::nVSPoints()
{
	return m_Param->m_nVSPoints;
}

float& cTaskChar::fExParam(int n)
{
	return m_Param->m_fExParam[n];
}

float cTaskChar::fDefWeight()
{
	return m_pDefParam->m_fWeight;
}

uint cTaskChar::nDefHereticVal()
{
	return m_pDefParam->m_nHereticVal;
}

float cTaskChar::fDefMaxSpeed()
{
	return m_pDefParam->m_fMaxSpeed;
}

float cTaskChar::fDefAccelSpeed()
{
	return m_pDefParam->m_fAccelSpeed;
}

float cTaskChar::fDefDecelSpeed()
{
	return m_pDefParam->m_fDecelSpeed;
}

float cTaskChar::fDefHitPower()
{
	return m_pDefParam->m_fHitPower;
}

float cTaskChar::fDefChargePower()
{
	return m_pDefParam->m_fChargePower;
}

float cTaskChar::fDefSlopePower()
{
	return m_pDefParam->m_fSlopePower;
}

float cTaskChar::fDefWallReflect()
{
	return m_pDefParam->m_fWallReflect;
}

float cTaskChar::fDefHitSize()
{
	return m_pDefParam->m_fHitSize;
}

float cTaskChar::fDefWHitSize()
{
	return m_pDefParam->m_fWHitSize;
}

int cTaskChar::nDefPoints()
{
	return m_pDefParam->m_nPoints;
}

int cTaskChar::nDefVSPoints()
{
	return m_pDefParam->m_nVSPoints;
}

float cTaskChar::fDefExParam(int n)
{
	return m_pDefParam->m_fExParam[n];
}

Vector4& cTaskChar::vSpeed()
{
	return m_vSpeed;
}

float& cTaskChar::fSpeed()
{
	return m_fSpeed;
}

ushort& cTaskChar::nInputDir()
{
	return m_nInputDir;
}

ushort& cTaskChar::nModelDir()
{
	return m_nModelDir;
}

ushort& cTaskChar::nFallDir()
{
	return m_nFallDir;
}

Vector4& cTaskChar::vHSpeed()
{
	return m_vHSpeed;
}

uint& cTaskChar::nHereticTimerBase()
{
	return m_nHereticTimerBase;
}

uint& cTaskChar::nHereticTimer()
{
	return m_nHereticTimer;
}

float cTaskChar::HereticRate()
{
	if (m_nHereticTimerBase == 0)
	{
		return 0.0f;
	}
	return (float)m_nHereticTimer / (float)m_nHereticTimerBase;
}

void cTaskChar::SetHeresy(uint time)
{
	OnFlag(1024u);
	m_nHereticTimerBase = (m_nHereticTimer = time);
}

int& cTaskChar::nLastRefID()
{
	return m_nLastRefID;
}

uint& cTaskChar::nLastRefCount()
{
	return m_nLastRefCount;
}

uint& cTaskChar::RefFlag()
{
	return m_nRefFlag;
}

bool cTaskChar::bRefFlag()
{
	return m_nRefFlag != 0;
}

int& cTaskChar::RefChar()
{
	return m_nRefChar;
}

int& cTaskChar::RefCharOLD()
{
	return m_nRefCharOLD;
}

int& cTaskChar::RefAxisOLD()
{
	return m_nRefAxisOLD;
}

int& cTaskChar::RefParts()
{
	return m_nRefParts;
}

bool cTaskChar::bRefParts()
{
	return m_nRefParts != 0;
}

int& cTaskChar::RefPartsOLD()
{
	return m_nRefPartsOLD;
}

bool cTaskChar::bRefPartsOLD()
{
	return m_nRefPartsOLD != 0;
}

int& cTaskChar::RefCounter()
{
	return m_nRefCounter;
}

Constants::MO_CHAR_REFLECT* cTaskChar::RefHistory(int n)
{
	return m_RefHistory[n];
}

void cTaskChar::RefAttacked(int nParts, int nGrobalID)
{
}

void cTaskChar::RefAttack(int nParts, int nGrobalID)
{
}

uint& cTaskChar::nHitFlag()
{
	return m_nHitFlag;
}

bool cTaskChar::bnHitFlag()
{
	return m_nHitFlag != 0;
}

uint& cTaskChar::nHitFlagAll()
{
	return m_nHitFlagAll;
}

int cTaskChar::nExHitNum()
{
	return m_nExHitNum;
}

bool cTaskChar::bnExHitNum()
{
	return m_nExHitNum != 0;
}

Constants::EX_HIT_PARAM* cTaskChar::ExHitParam(int n)
{
	if (m_pExHitParam.empty())
	{
		return nullptr;
	}
	if (n >= m_nExHitNum)
	{
		return nullptr;
	}
	return m_pExHitParam[n];
}

Constants::PANEL_INFO* cTaskChar::pUGPanel(int n)
{
	return m_pUGPanels[n];
}

float cTaskChar::GetHitPower(int nPart, int nOtherID)
{
	return fHitPower();
}

float cTaskChar::GetChargePower(int nPart, int nOtherID)
{
	return fChargePower();
}

Constants::CHAR_GIVE_POINTS*& cTaskChar::pGivePoints()
{
	return m_GivePoints;
}

int& cTaskChar::nPiyoTimer()
{
	return m_nPiyoTimer;
}

short& cTaskChar::nModel()
{
	return m_nModel;
}

ushort& cTaskChar::nEffectTimer()
{
	return m_nEffectTimer;
}

void cTaskChar::RegistPacket()
{
}

void cTaskChar::SyncPacket()
{
}

int& cTaskChar::nChipColor()
{
	return m_nChipColor;
}

void cTaskChar::FallEffect(int nRefID)
{
}

int& cTaskChar::nMutekiTimer()
{
	return m_nMutekiTimer;
}

bool cTaskChar::IsFall(float x, float y, float z)
{
	return IsFall(Vector4(x, y, z, 0.0f));
}

bool cTaskChar::TaikiAction()
{
	return false;
}

uint cTaskChar::Input()
{
	return 0u;
}

void cTaskChar::SetMMSParam(int para1, int para2, int para3)
{
}

void cTaskChar::DeleteReq()
{
	OnFlag(8388608u);
}

void cTaskChar::SetFrame(uint model, float frame)
{
}

float cTaskChar::GetFrame(uint model)
{
	return 0.0f;
}

bool cTaskChar::AnimateModel(uint model)
{
	if (pModel(model)->GetFrameAnmChr(0u) >= pModel(model)->GetFrameMaxAnmChr(0u))
	{
		return true;
	}
	return false;
}

// DOL 0x801e2a6c [audited: default param struct is s_DefParam->Clone()]
void cTaskChar::Init()
{
	m_nGrobalID = 0u;
	m_nID = 0u;
	m_nKind = 0u;
	m_nCharFlag = 0u;
	m_nCharFlagOLD = 0u;
	bForceDeath() = false;
	for (int i = 0; i < 4; i++)
	{
		m_nStep[i] = 0;
	}
	m_phEffPiyo = nullptr;
	m_nMode = 0;
	m_nOldMode = 0;
	m_nNextMode = 0;
	m_nReturnMode = 0;
	m_nTimer = 0;
	m_Param = s_DefParam->Clone();
	m_pDefParam = s_DefParam;
	::Constants::jetGeoVecUnit(m_vSpeed);
	m_fSpeed = 0.0f;
	m_nInputDir = 0;
	m_nModelDir = std::numeric_limits<ushort>::max();
	m_nFallDir = 0;
	::Constants::jetGeoVecUnit(m_vHSpeed);
	m_nHereticTimer = 0u;
	m_nHereticTimerBase = 0u;
	m_nLastRefID = -1;
	m_nRefFlag = 0u;
	m_nRefChar = -1;
	m_nRefCharOLD = -1;
	m_nRefAxisOLD = -1;
	m_nRefParts = -1;
	m_nRefPartsOLD = -1;
	m_nRefCounter = 0;
	m_nRefHistIndex = 0;
	m_bMtxUsed = false;
	::Constants::jetGeoMtxIdentity(m_mMtx);
	for (int j = 0; j < (int)std::size(m_RefHistory); j++)
	{
		m_RefHistory[j] = new Constants::MO_CHAR_REFLECT();
	}
	for (int k = 0; k < 60; k++)
	{
		m_RefHistory[k]->m_nIndex = (ushort)k;
	}
	m_nHitFlag = 1u;
	m_nHitFlagAll = 1u;
	m_nExHitNum = 0;
	m_pExHitParam.clear();
	for (int l = 0; l < 5; l++)
	{
		m_pUGPanels[l] = nullptr;
	}
	m_nPiyoTimer = 0;
	m_GivePoints = new Constants::CHAR_GIVE_POINTS();
	m_nModel = 0;
	m_nEffectTimer = 0;
	m_nChipColor = 8;
	m_nMutekiTimer = 0;
	::Constants::jetGeoVecUnit(m_vMTrans);
	::Constants::jetGeoVecUnit(m_vMTransOld);
	SetName("cTaskChar");
}

void cTaskChar::Term()
{
	m_pExHitParam.clear();
	cTaskModel::Term();
}

// DOL 0x801e2d9c [audited: audited 44th] (audited 44th: matches)
bool cTaskChar::Execute(cJetTask* pParam)
{
	if (bForceDeath())
	{
		ChangeTaiki();
	}
	if (!Constants::IS_MYCHAR(this))
	{
		FlagBackup();
		SyncPacket();
	}
	if (!Constants::MOTOS_IS_PAUSE() && !Constants::IS_CLEARPAUSE())
	{
		if (!bTestFlag(16384u))
		{
			Move();
		}
		if (CHECK_DELETE_REQ())
		{
			return true;
		}
		Status();
		if (Constants::IS_MYCHAR(this))
		{
			FlagBackup();
		}
		if (CHECK_DELETE_REQ())
		{
			return true;
		}
		if (!bTestFlag(16384u))
		{
			ActionBoth();
			if (Constants::IS_MYCHAR(this))
			{
				ActionLocal();
			}
			else
			{
				ActionOther();
			}
		}
		else
		{
			TaikiAction();
		}
		if (CHECK_DELETE_REQ())
		{
			return true;
		}
	}
	if (Constants::IS_MYCHAR(this))
	{
		RegistPacket();
	}
	if (CHECK_DELETE_REQ())
	{
		return true;
	}
	UpdateMatrix();
	UpdateFrame();
	return true;
}

// DOL 0x801e3048 [audited]
// DOL 0x801e3048 (audited 44th: matches)
void cTaskChar::Draw(int nStatus, cJetTask* pParam)
{
	uint num = 0u;
	while (true)
	{
		cTaskModel::cPBModel* cPBModel = pPBModel(num);
		if (cPBModel == nullptr)
		{
			break;
		}
		Matrix4x4 m = ::Constants::scMtxIdentity;
		if (m_bMtxUsed)
		{
			if (CustomKingdomUtil::IF(KingdomGlobal::g_pMotosGame->fSlopeRad()))
			{
				::Constants::jetGeoMtxMult(m, m, KingdomGlobal::g_pMotosGame->pSlopeMatrix());
			}
			m *= Matrix4x4::Translate(vMTrans());
			m *= m_mMtx;
		}
		else
		{
			m = KingdomGlobal::g_pMotosGame->pSlopeMatrix() * Matrix4x4::Translate(cPBModel->vTrans());
			::Constants::jetGeoMtxRotZYX(m, m, cPBModel->vRotate());
		}
		DummyNW4R::SetMtx(cPBModel->pScnMdl(), m);
		DummyNW4R::SetScale(cPBModel->pScnMdl(), cPBModel->vScale()[0], cPBModel->vScale()[1], cPBModel->vScale()[2]);
		cPBModel->UpdateMatrix();   // DOL 0x801e3204 [audited]: the push-back (+0xd8) into this frame's scene list (0x8009c3f4)
		num++;
	}
}

void cTaskChar::UpdateFrame()
{
	if (!Constants::MOTOS_IS_PAUSE())
	{
		cTaskModel::UpdateFrame();
	}
}

Vector4& cTaskChar::vWTrans()
{
	CustomKingdomUtil::Assert(pModel() != nullptr, "nullpo");
	return pModel()->vTrans();
}

Vector4& cTaskChar::vMTrans()
{
	return m_vMTrans;
}

Vector4& cTaskChar::vMTransOLD()
{
	return m_vMTransOld;
}

Vector4& cTaskChar::vRotate()
{
	return m_vRotate;
}

Vector4& cTaskChar::vScale()
{
	CustomKingdomUtil::Assert(pModel() != nullptr, "nullpo");
	return pModel()->vScale();
}

void cTaskChar::UpdateMatrix()
{
	cKingdomModel* cKingdomModel2;
	if ((cKingdomModel2 = pModel()) != nullptr)
	{
		::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), m_vMTrans);
		::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), m_vRotate);
	}
}

void cTaskChar::RollMatrix(Matrix4x4& _out, int model, const Vector4& speed, const string& nodename)
{
	Vector4 v = Vector4();
	Matrix4x4 _discard1;
	::Constants::jetGeoMtxIdentity(_discard1);
	Matrix4x4 m2;
	::Constants::jetGeoMtxIdentity(m2);
	m_bMtxUsed = true;
	Vector4 _out2;
	Constants::VEC3_SET(_out2, speed[0], 0.0f, speed[2]);
	float num = Constants::tspVector3Length(_out2);
	if (CustomKingdomUtil::IF(num))
	{
		pModel()->GetLocalNodeTrans(v, nodename);
		float num2 = (_out[2][3] = 0.0f);
		float value = (_out[1][3] = num2);
		_out[0][3] = value;
		::Constants::jetGeoVecNormalize(_out2, _out2);
		::Constants::jetGeoVecOuterProduct(_out2, Constants::VEC3_GET(0.0f, 1.0f, 0.0f), _out2);
		float num5 = (float)std::numbers::pi * 2.0f * v[1];
		if (CustomKingdomUtil::IF(num5))
		{
			::Constants::jetGeoMtxRotAxis(m2, m2, _out2, (float)std::numbers::pi * 2.0f * (num / num5));
		}
		::Constants::jetGeoMtxMult(_out, m2, _out);
		Constants::VEC3_SET(_out2, 0.0f - v[0], 0.0f - v[1], 0.0f - v[2]);
		::Constants::jetGeoVecTrans(_out2, _out, _out2);
		_out[0][3] = _out2[0] + v[0];
		_out[1][3] = _out2[1] + v[1];
		_out[2][3] = _out2[2] + v[2];
	}
}

void cTaskChar::RollMatrix(Matrix4x4& _out, int model, const string& nodename)
{
	RollMatrix(_out, model, vSpeed(), nodename);
}

void cTaskChar::SetDrawRollMatrix(Vector4 trans, Matrix4x4& roll, int model)
{
	Matrix4x4 m = Matrix4x4();
	if (KingdomGlobal::g_pMotosGame->pMap() != nullptr)
	{
		KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWTrans(), trans);
	}
	else
	{
		vWTrans() = trans;
	}
	if (CustomKingdomUtil::IF(KingdomGlobal::g_pMotosGame->fSlopeRad()))
	{
		::Constants::jetGeoMtxMult(m, m, KingdomGlobal::g_pMotosGame->pSlopeMatrix());
	}
	::Constants::jetGeoMtxTrans(m, m, vWTrans());
	::Constants::jetGeoMtxMult(m, m, roll);
}

void cTaskChar::SetDrawRollMatrix(Matrix4x4& roll, int model)
{
}

// DOL 0x801e3718 [audited]
bool cTaskChar::Move()
{
	if ((m_nCharFlag & 0x40) == 0)
	{
		if (Constants::IS_MYCHAR(this))
		{
			NormalMove();
		}
	}
	else
	{
		FallMove();
	}
	return true;
}

// DOL 0x801e378c [audited: audited 44th] (audited 44th: matches)
bool cTaskChar::Status()
{
	UpdateUGPanel(vMTrans());
	if (!bTestFlag(16384u))
	{
		if (CheckFall(vMTrans()))
		{
			if (!bTestFlag(131072u))
			{
				Vector4 pOut = Vector4();
				int dir = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut, vMTrans(), fWHitSize());
				nFallDir() = (ushort)Constants::motosReverseDir(dir);
			}
			OnFlag(131072u);
		}
		else
		{
			OffFlag(131072u);
		}
	}
	if (pGivePoints()->m_nGameCounter != 0 && Constants::get_GAME_COUNTER() >= pGivePoints()->m_nGameCounter + 256)
	{
		pGivePoints() = new Constants::CHAR_GIVE_POINTS();
	}
	if (nMutekiTimer() != 0 && --nMutekiTimer() == 0)
	{
		OffFlag(262144u);
	}
	if (!bTestFlag(2048u))
	{
		if (bTestFlag(256u))
		{
			nHitFlag() = 4u;
		}
		else if (bTestFlag(128u))
		{
			nHitFlag() = 2u;
		}
		else
		{
			nHitFlag() = 1u;
		}
	}
	nHitFlagAll() = nHitFlag();
	for (int i = 0; i < nExHitNum(); i++)
	{
		nHitFlagAll() |= ExHitParam(i)->m_nFlag;
	}
	if ((!bTestFlag(8u) && nKind() != 22 && nKind() != 23 && nKind() != 14 && nKind() != 15) ? true : false)
	{
		if (nPiyoTimer() > 0 && --nPiyoTimer() != 0)
		{
			OnFlag(2097152u);
		}
		else
		{
			nPiyoTimer() = 0;
			OffFlag(2097152u);
			if (m_phEffPiyo != nullptr)
			{
				m_phEffPiyo->Detach(true);
				m_phEffPiyo = nullptr;
			}
		}
		if (bTrigFlag(2097152u))
		{
			m_phEffPiyo = Constants::SpawnEffect(33, vMTrans(), this, 0, false);
		}
	}
	else
	{
		nPiyoTimer() = 0;
	}
	UpdateRefHistory();
	if (vMTrans()[2] >= 50.0f)
	{
		ChangeTaiki();
	}
	else if (bTestFlag(16384u))
	{
		OffFlag(16385u);
		vMTrans()[1] = 0.0f;
	}
	if (!bTestFlagOLD(64u) && bTestFlag(64u) && KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		if (bTestFlag(2u))
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE030_DEAD);
		}
		else
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE030_DEAD);
		}
	}
	return true;
}

bool cTaskChar::ActionBoth()
{
	return true;
}

bool cTaskChar::ActionLocal()
{
	nInputDir() = (ushort)Input();
	Dir2Rotate(nModelDir());
	return true;
}

bool cTaskChar::ActionOther()
{
	Dir2Rotate(nModelDir());
	return true;
}

// DOL 0x801e3bd0 [audited: per AUDIT_G03_G07 row] (audited 44th: matches)
void cTaskChar::NormalMove()
{
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	Vector4 v3 = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	::Constants::jetGeoVecUnit(v2);
	::Constants::jetGeoVecUnit(v3);
	::Constants::jetGeoVecCopy(vMTransOLD(), vMTrans());
	if (nHereticTimer() != 0)
	{
		if (--nHereticTimer() == 0)
		{
			if (fChargePower() > fDefChargePower())
			{
				fChargePower() = fDefChargePower();
			}
			OffFlag(4096u);
			OffFlag(1048576u);
		}
	}
	else
	{
		OffFlag(1024u);
		OffFlag(4096u);
		OffFlag(1048576u);
	}
	if (!bTestFlag(384u) && (double)HereticRate() < 0.3 && KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->vSlopePower());
		v[0] *= fSlopePower();
		Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
	}
	AdjustMaxSpeed();
	CalcSpeed(v);
	if ((double)Constants::VEC3_LENGTH_SQUARE(v[0], v[1], v[2]) > 1.0)
	{
		Vector4 v4;
		::Constants::jetGeoVecCopy(v4, v);
		float num = Constants::tspVector3Length(v);
		do
		{
			::Constants::jetGeoVecNormalize(v4, v4);
			if ((double)num > 0.5)
			{
				Constants::VEC3_MUL(v4, v4, 0.5f);
				num -= 0.5f;
			}
			else
			{
				Constants::VEC3_MUL(v4, v4, num);
				num = 0.0f;
			}
			if (MapMove(v2, v3, v4, v4))
			{
				if (nHereticTimer() == 0)
				{
					::Constants::jetGeoVecCopy(vSpeed(), v4);
				}
				else
				{
					Constants::VEC3_MUL(vHSpeed(), v4, HereticRate());
					Constants::VEC3_MUL(vSpeed(), v4, 1.0f - HereticRate());
				}
			}
			Constants::VEC3_ADD(vMTrans(), v2, v3);
		}
		while (CustomKingdomUtil::IF(num));
	}
	else
	{
		if (MapMove(v2, v3, v, v))
		{
			if (nHereticTimer() == 0)
			{
				::Constants::jetGeoVecCopy(vSpeed(), v);
			}
			else
			{
				Constants::VEC3_MUL(vHSpeed(), v, HereticRate());
				Constants::VEC3_MUL(vSpeed(), v, 1.0f - HereticRate());
			}
		}
		Constants::VEC3_ADD(vMTrans(), v2, v3);
	}
	DecelSpeed(nInputDir());
}

// DOL 0x801e4180 [audited]
bool cTaskChar::FallMove()
{
	::Constants::jetGeoVecCopy(vMTransOLD(), vMTrans());
	int num = 0;
	Vector4 pOut = Vector4();
	Vector4 _out;
	switch (Step())
	{
	case 0:
		if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut, vMTrans(), fWHitSize()) != 0)
		{
			Constants::VEC3_SUB(_out, pOut, vMTrans());
			_out[1] = 0.0f;
			float num2 = ::Constants::jetGeoVecLength(_out);
			if ((double)num2 > 0.1)
			{
				Constants::VEC3_DIV(_out, _out, num2);
				Constants::VEC3_MUL(_out, _out, 0.1f);
			}
			vSpeed()[0] = _out[0];
			vSpeed()[2] = _out[2];
		}
		else
		{
			vSpeed()[0] = 0.0f;
			vSpeed()[2] = 0.0f;
			Step()++;
			Step(1) = 0;
		}
		vSpeed()[1] = 0.0f;
		break;
	case 1:
		::Constants::jetGeoVecCopy(_out, KingdomGlobal::g_pMotosGame->pGravVector());
		Constants::VEC3_MUL(_out, _out, 0.01f);
		Constants::VEC3_ADD(vSpeed(), vSpeed(), _out);
		if (vRotate()[1] > (float)std::numbers::pi)
		{
			vRotate()[1] += 0.1f;
		}
		else
		{
			vRotate()[1] -= 0.1f;
		}
		break;
	}
	Constants::VEC3_ADD(vMTrans(), vMTrans(), vSpeed());
	if (Constants::IS_MYCHAR(this) && vMTrans()[1] < -40.0f)
	{
		vMTrans()[2] = 50.0f;
	}
	return false;
}

// DOL 0x801e43ec [audited]
void cTaskChar::ChangeFallMove()
{
	if (bTestFlag(64u))
	{
		return;
	}
	vSpeed()[0] = 0.0f;
	vSpeed()[1] = 0.0f;
	vSpeed()[2] = 0.0f;
	Step() = 0;
	OnFlag(64u);
	if (!bTestFlag(2u) && bTestFlag(1024u))
	{
		int num = nLastRefID();
		if ((num & 0x80) != 0)
		{
			Constants::MO_CHAR_REFLECT* mO_CHAR_REFLECT = RefHistoryOffset(2);
			if (mO_CHAR_REFLECT->m_nRefID != 0 && (int)mO_CHAR_REFLECT->m_nCount > (int)(Constants::get_GAME_COUNTER() - 60))
			{
				num = (int)mO_CHAR_REFLECT->m_nOtherID;
			}
		}
		ExecGivePoints(num);
		FallEffect(num);
	}
	if (!bTestFlag(2u) && !KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		CustomKingdomUtil::Assert(KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr, "nullpo");
		if (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(64u) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16384u))
		{
			KingdomGlobal::MotosSys->bMotosCharDead(GrobalID()) = true;
		}
	}
	nLastRefID() = -1;
}

// DOL 0x801e4650 [audited: audited 44th] (audited 44th: matches)
void cTaskChar::ChangeTaiki()
{
	if (!bTestFlag(16384u))
	{
		// DOL 0x801e4650: no kill count here (the C#'s AddDeadEnemyCnt was a Steam statistic)
		OnFlag(16384u);
		OffFlag(19005888u);
		nPiyoTimer() = 0;
		Step() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		nHereticTimer() = 0u;
		vMTrans()[0] = (float)GrobalID() * 20.0f;
		vMTrans()[1] = 0.0f;
		vMTrans()[2] = 51.0f;
		vRotate()[1] = 0.0f;
	}
}

// DOL 0x801e46dc [audited: audited 44th] (audited 44th: matches)
void cTaskChar::ExecGivePoints(int id, int add_point, int sub_point)
{
	if (id == -1 || (id & 0x80) != 0 || !Constants::IS_MYCHAR(this))
	{
		return;
	}
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)id);
	if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(2u))
	{
		pGivePoints()->m_nTargetID = (short)cTaskChar2->ID();
		if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
		{
			pGivePoints()->m_nAddPoints = (ushort)nPoints();
		}
		else
		{
			pGivePoints()->m_nAddPoints = (ushort)nVSPoints();
		}
		pGivePoints()->m_nSubPoints = 0;
		pGivePoints()->m_nGameCounter = Constants::get_GAME_COUNTER();
	}
}

// DOL 0x801e47a8 [audited: per AUDIT_G03_G07 row] (pOutSpeed is only written when something was hit: the C# `out` parameter zeroed it first, which made the second step of
// NormalMove's multi-step loop normalise a zero vector when the first step hit nothing)
bool cTaskChar::MapMove(Vector4& pOutTrans, Vector4& pOutVec, Vector4& pOutSpeed, Vector4 pVec)
{
	MAP_HIT_RESULT* mAP_HIT_RESULT = nullptr;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pVec);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, vMTrans());
	Vector4 _out = Vector4();
	float num = Constants::tspVector3Length(pVec);
	int num2 = 0;
	int num3 = 0;
	int num4 = 0;
	int num5 = 10;
	v2[1] = 0.0f;
	if (!bTestFlag(384u))
	{
		do
		{
			num4 = KingdomGlobal::g_pMotosGame->pMap()->HitCheck(v2, v, fWHitSize());
			num3 = 0;
			for (int i = 0; i < num4; i++)
			{
				bool flag = false;
				mAP_HIT_RESULT = KingdomGlobal::g_pMotosGame->pMap()->HitResult(i);
				if (!CustomKingdomUtil::IF(num3 & 5) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 1))
				{
					flag = true;
				}
				else if (!CustomKingdomUtil::IF(num3 & 6) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 2))
				{
					flag = true;
				}
				else if (!CustomKingdomUtil::IF(num3 & 3) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 4))
				{
					flag = true;
				}
				if (flag)
				{
					if (mAP_HIT_RESULT->m_nFlag != 3)
					{
						Vector4 _out2;
						Constants::VEC3_ADD(_out2, v2, v);
						float num6 = Constants::Motos_P_vecDistance(_out2, mAP_HIT_RESULT->m_vCrs);
						Constants::Motos_P_vecReflect(_out2, mAP_HIT_RESULT->m_vNormal, v);
						Constants::Motos_P_vecNormalize(_out, _out2);
						if (num5 == 10)
						{
							Constants::VEC3_MUL(_out2, _out, num6 * fWallReflect() + 0.05f);
						}
						else
						{
							Constants::VEC3_MUL(_out2, _out, num6 * 0.7f + 0.05f);
						}
						if (CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 1))
						{
							if (Constants::MOTOS_P_ABS(_out2[0]) < 0.05f)
							{
								_out2[0] = ((_out2[0] >= 0.0f) ? 0.05f : (-0.05f));
							}
						}
						else if (CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 2))
						{
							if ((double)Constants::MOTOS_P_ABS(_out2[2]) < 0.05)
							{
								_out2[2] = ((_out2[2] >= 0.0f) ? 0.05f : (-0.05f));
							}
						}
						else
						{
							CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 4);
						}
						::Constants::jetGeoVecCopy(v2, mAP_HIT_RESULT->m_vCrs);
						::Constants::jetGeoVecCopy(v, _out2);
					}
					else
					{
						Vector4 _out3;
						Constants::VEC3_SET(_out3, mAP_HIT_RESULT->m_vNormal[0], 0.0f, 0.0f);
						Constants::Motos_P_vecReflect(v, _out3, v);
						Constants::VEC3_SET(_out3, 0.0f, 0.0f, mAP_HIT_RESULT->m_vNormal[2]);
						Constants::Motos_P_vecReflect(v, _out3, v);
						Constants::Motos_P_vecNormalize(_out, v);
					}
				}
				num3 |= (int)mAP_HIT_RESULT->m_nFlag;
				num2 |= (int)mAP_HIT_RESULT->m_nFlag;
			}
		}
		while (CustomKingdomUtil::IF(num4) && CustomKingdomUtil::IF(--num5));
		CustomKingdomUtil::IF(num5);
	}
	if (CustomKingdomUtil::IF(num2))
	{
		::Constants::jetGeoVecCopy(pOutTrans, v2);
		::Constants::jetGeoVecCopy(pOutVec, v);
		Constants::VEC3_MUL(pOutSpeed, _out, num);
		if (bTestFlag(2u) && num > fDefMaxSpeed())
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE005_HIT_WALL);
		}
		OnFlag(16777216u);
		return true;
	}
	::Constants::jetGeoVecCopy(pOutTrans, vMTrans());
	::Constants::jetGeoVecCopy(pOutVec, pVec);
	OffFlag(16777216u);
	return false;
}

bool cTaskChar::MapMove(Vector4& pOut, Vector4 pVec)
{
	pOut = Vector4();
	MAP_HIT_RESULT* mAP_HIT_RESULT = nullptr;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pVec);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, vMTrans());
	Vector4 _out = Vector4();
	float num = Constants::tspVector3Length(pVec);
	int num2 = 0;
	int num3 = 0;
	int num4 = 0;
	int num5 = 10;
	v2[1] = 0.0f;
	if (!bTestFlag(384u))
	{
		do
		{
			num4 = KingdomGlobal::g_pMotosGame->pMap()->HitCheck(v2, v, fWHitSize());
			num3 = 0;
			for (int i = 0; i < num4; i++)
			{
				bool flag = false;
				mAP_HIT_RESULT = KingdomGlobal::g_pMotosGame->pMap()->HitResult(i);
				if (!CustomKingdomUtil::IF(num3 & 5) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 1))
				{
					flag = true;
				}
				else if (!CustomKingdomUtil::IF(num3 & 6) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 2))
				{
					flag = true;
				}
				else if (!CustomKingdomUtil::IF(num3 & 3) && CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 4))
				{
					flag = true;
				}
				if (flag)
				{
					if (mAP_HIT_RESULT->m_nFlag != 3)
					{
						Vector4 _out2;
						Constants::VEC3_ADD(_out2, v2, v);
						float num6 = Constants::Motos_P_vecDistance(_out2, mAP_HIT_RESULT->m_vCrs);
						Constants::Motos_P_vecReflect(_out2, mAP_HIT_RESULT->m_vNormal, v);
						Constants::Motos_P_vecNormalize(_out, _out2);
						if (num5 == 10)
						{
							Constants::VEC3_MUL(_out2, _out, num6 * fWallReflect() + 0.05f);
						}
						else
						{
							Constants::VEC3_MUL(_out2, _out, num6 * 0.7f + 0.05f);
						}
						if (CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 1))
						{
							if (Constants::MOTOS_P_ABS(_out2[0]) < 0.05f)
							{
								_out2[0] = ((_out2[0] >= 0.0f) ? 0.05f : (-0.05f));
							}
						}
						else if (CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 2))
						{
							if (Constants::MOTOS_P_ABS(_out2[2]) < 0.05f)
							{
								_out2[2] = ((_out2[2] >= 0.0f) ? 0.05f : (-0.05f));
							}
						}
						else if (CustomKingdomUtil::IF(mAP_HIT_RESULT->m_nFlag & 4))
						{
							if (Constants::MOTOS_P_ABS(_out2[0]) < 0.05f)
							{
								_out2[0] = ((_out2[0] >= 0.0f) ? 0.05f : (-0.05f));
							}
							if (Constants::MOTOS_P_ABS(_out2[2]) < 0.05f)
							{
								_out2[2] = ((_out2[2] >= 0.0f) ? 0.05f : (-0.05f));
							}
						}
						::Constants::jetGeoVecCopy(v2, mAP_HIT_RESULT->m_vCrs);
						::Constants::jetGeoVecCopy(v, _out2);
					}
					else
					{
						Vector4 _out3;
						Constants::VEC3_SET(_out3, mAP_HIT_RESULT->m_vNormal[0], 0.0f, 0.0f);
						Constants::Motos_P_vecReflect(v, _out3, v);
						Constants::VEC3_SET(_out3, 0.0f, 0.0f, mAP_HIT_RESULT->m_vNormal[2]);
						Constants::Motos_P_vecReflect(v, _out3, v);
						Constants::Motos_P_vecNormalize(_out, v);
					}
				}
				num3 |= (int)mAP_HIT_RESULT->m_nFlag;
				num2 |= (int)mAP_HIT_RESULT->m_nFlag;
			}
		}
		while (CustomKingdomUtil::IF(num4) && CustomKingdomUtil::IF(--num5));
		CustomKingdomUtil::IF(num5);
	}
	if (CustomKingdomUtil::IF(num2))
	{
		Constants::VEC3_ADD(vMTrans(), v2, v);
		Constants::VEC3_MUL(pOut, _out, num);
		if (bTestFlag(2u) && num > fDefMaxSpeed())
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE005_HIT_WALL);
		}
		return true;
	}
	Constants::VEC3_ADD(vMTrans(), vMTrans(), pVec);
	return false;
}

void cTaskChar::PresetEnv()
{
}

// DOL 0x801e5144 [audited]
void cTaskChar::InitParam(MO_CHAR_PARAMS* pParam)
{
	m_Param = pParam->Clone();
	m_pDefParam = pParam;
}

int cTaskChar::UpdateUGPanel(Nullable<Vector4> pVec)
{
	if (!pVec.has_value())
	{
		pVec = vMTrans();
	}
	m_pUGPanels[0] = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanel(pVec.value());
	return 5;
}

bool cTaskChar::IsFall(Nullable<Vector4> pVec)
{
	if (pVec.has_value())
	{
		return KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(pVec.value());
	}
	return bTestFlag(131072u);
}

// DOL 0x801e521c [audited: per AUDIT_G03_G07 row]
bool cTaskChar::CheckFall(Vector4 pVec)
{
	bool result = false;
	if (bTestFlag(2048u))
	{
		int array[4] = {};
		int num = 0;
		int num2 = 0;
		if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(vMTrans()))
		{
			float num3 = ((nKind() != 24 && nKind() != 26) ? (KingdomGlobal::g_pMotosGame->pMap()->fPanelSize() / 2.0f) : KingdomGlobal::g_pMotosGame->pMap()->fPanelSize());
			for (int i = 0; i < 4; i++)
			{
				Vector4 v;
				::Constants::jetGeoVecCopy(v, vMTrans());
				switch (i)
				{
				case 0:
					v[0] -= num3;
					v[2] -= num3;
					break;
				case 1:
					v[0] += num3;
					v[2] -= num3;
					break;
				case 2:
					v[0] -= num3;
					v[2] += num3;
					break;
				case 3:
					v[0] += num3;
					v[2] += num3;
					break;
				}
				if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v))
				{
					array[i] = 0;
				}
				else if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(v))
				{
					array[i] = 1;
					num2++;
				}
				else
				{
					num++;
					array[i] = 1;
				}
				if (i > 1 && CustomKingdomUtil::IF(array[i]) && CustomKingdomUtil::IF(array[(i - 2) ^ 1]))
				{
					num++;
				}
			}
			if (!CustomKingdomUtil::IF(num) || (num < 2 && num2 < 2))
			{
				result = true;
			}
		}
	}
	else if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(vMTrans()))
	{
		result = true;
	}
	return result;
}

// DOL 0x801e5484 [audited: per AUDIT_G03_G07 row]
void cTaskChar::AdjustMaxSpeed(Vector4& pVec)
{
	float num = Constants::tspVector3Length(pVec);
	if (num > fMaxSpeed())
	{
		num = fMaxSpeed() / num;
		Constants::VEC3_MUL(pVec, pVec, num);
	}
	fSpeed() = num;
}

void cTaskChar::AdjustMaxSpeed()
{
	AdjustMaxSpeed(vSpeed());
}

// DOL 0x801e5524 [audited: per AUDIT_G03_G07 row]
void cTaskChar::DecelSpeed(int input_dir)
{
	if ((input_dir & 0xC) == 0)
	{
		vSpeed()[0] *= fDecelSpeed();
	}
	if ((input_dir & 3) == 0)
	{
		vSpeed()[2] *= fDecelSpeed();
	}
}

// DOL 0x801e5558 [audited: record field order +2 id,+4 otherID,+8 flag,+0xc count assumed from MO_CHAR_REFLECT layout]
void cTaskChar::RegistRefHistory(int id, int count, uint flag, int otherID, float pow, float charge)
{
	Constants::MO_CHAR_REFLECT* mO_CHAR_REFLECT = m_RefHistory[m_nRefHistIndex];
	if (mO_CHAR_REFLECT != nullptr)
	{
		mO_CHAR_REFLECT->m_nRefID = (ushort)id;
		mO_CHAR_REFLECT->m_nCount = (uint)count;
		mO_CHAR_REFLECT->m_nCharFlag = flag;
		mO_CHAR_REFLECT->m_nOtherID = (uint)otherID;
		mO_CHAR_REFLECT->m_fPower = pow;
		mO_CHAR_REFLECT->m_fCharge = charge;
	}
	m_nRefHistIndex++;
	while (m_nRefHistIndex >= 60)
	{
		m_nRefHistIndex -= 60;
	}
	m_RefHistory[m_nRefHistIndex]->m_nRefID = 0;
}

Constants::MO_CHAR_REFLECT* cTaskChar::SearchRefHistory(int id)
{
	for (int i = 0; i < 60; i++)
	{
		if (m_RefHistory[i]->m_nRefID == id)
		{
			return m_RefHistory[i];
		}
	}
	return nullptr;
}

void cTaskChar::UpdateRefHistory()
{
	for (int i = 0; i < 60; i++)
	{
		if ((int)m_RefHistory[i]->m_nCount < (int)(Constants::get_GAME_COUNTER() - 15))
		{
			m_RefHistory[i]->m_nRefID = 0;
		}
	}
}

// DOL 0x801e5600 [audited]
Constants::MO_CHAR_REFLECT* cTaskChar::RefHistoryOffset(int n)
{
	for (n = m_nRefHistIndex - n; n < 0; n += 60)
	{
	}
	while (n >= 60)
	{
		n -= 60;
	}
	return m_RefHistory[n];
}

// DOL 0x801e5730 [audited: audited 44th] (audited 44th: matches)
void cTaskChar::ExecReflect(NET_RF_PACKET* pPacket)
{
	if (SearchRefHistory(pPacket->m_nRefID) != nullptr)
	{
		return;
	}
	uint flag = TestFlag(std::numeric_limits<uint>::max());
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars(pPacket->m_nChar[0]);
	cTaskChar* cTaskChar3 = KingdomGlobal::g_pMotosGame->pChars(pPacket->m_nChar[1]);
	int num = pPacket->m_nParts & 0xF;
	int num2 = (pPacket->m_nParts >> 4) & 0xF;
	uint num3 = (uint)((pPacket->m_nAxis_Status >> 4) & 3);
	uint num4 = (uint)((pPacket->m_nAxis_Status >> 6) & 3);
	int nOtherID;
	int num5;
	cTaskChar* cTaskChar4;
	cTaskChar* cTaskChar5;
	int nParts;
	int nParts2;
	uint num6;
	if (pPacket->m_nChar[0] == GrobalID())
	{
		nOtherID = pPacket->m_nChar[0];
		num5 = pPacket->m_nChar[1];
		cTaskChar4 = cTaskChar2;
		cTaskChar5 = cTaskChar3;
		nParts = num;
		nParts2 = num2;
		num6 = num4;
	}
	else
	{
		nOtherID = pPacket->m_nChar[1];
		num5 = pPacket->m_nChar[0];
		cTaskChar4 = cTaskChar3;
		cTaskChar5 = cTaskChar2;
		nParts = num2;
		nParts2 = num;
		num6 = num3;
	}
	float num7;
	float pow = (num7 = KingdomGlobal::g_pMotosGame->GrobalID2HitPower(num5, nParts2, nOtherID, pPacket->m_nRefID));
	float num8;
	float charge = (num8 = KingdomGlobal::g_pMotosGame->GrobalID2ChargePower((uint)num5, nParts2, (uint)nOtherID, pPacket->m_nRefID, (int)(num6 & 1)));
	if (Constants::get_GAME_COUNTER() - 10 > Constants::GAME_COUNTER_FROMBYTE(pPacket->m_nCounter))
	{
		RegistRefHistory(pPacket->m_nRefID, (int)Constants::GAME_COUNTER_FROMBYTE(pPacket->m_nCounter), flag, num5, num7, num8);
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosAxis2Vector(pPacket->m_nAxis_Status & 0xF));
	if (cTaskChar4 == cTaskChar3)
	{
		Constants::VEC3_MUL(v, v, -1.0f);
	}
	if (cTaskChar4 != nullptr && cTaskChar4 == cTaskChar2 && cTaskChar2 != nullptr && cTaskChar3 != nullptr && !cTaskChar2->bTestFlag(8u) && !cTaskChar3->bTestFlag(8u))
	{
		Constants::SpawnColEffect(cTaskChar2, cTaskChar3);
	}
	if (cTaskChar4 != nullptr && (cTaskChar5 == nullptr || (cTaskChar5 != nullptr && !cTaskChar5->bTestFlag(16u))))
	{
		cTaskChar4->nLastRefID() = num5;
	}
	cTaskChar4->nLastRefCount() = Constants::get_GAME_COUNTER();
	if (cTaskChar4 != nullptr && Constants::IS_MYCHAR(cTaskChar4))
	{
		RefAttack(nParts2, num5);
		RefAttacked(nParts, num5);
		OffFlag(4096u);
	}
	else
	{
		cTaskChar4 = nullptr;
	}
	if (cTaskChar4 != nullptr && cTaskChar5 != nullptr)
	{
		if (cTaskChar4->bTestFlag(2u) && cTaskChar5->bTestFlag(16u) && Constants::IS_MYCHAR(cTaskChar4))
		{
			cTaskChar4->GetItem((int)cTaskChar5->GrobalID());
			cTaskChar4 = nullptr;
		}
		else if (cTaskChar4->bTestFlag(16u) && cTaskChar5->bTestFlag(2u) && Constants::IS_MYCHAR(cTaskChar4))
		{
			cTaskChar4->vMTrans()[2] = 50.0f;
			cTaskChar4 = nullptr;
		}
	}
	if (cTaskChar4 != nullptr)
	{
		if (CustomKingdomUtil::IF(vSpeed()[0]) || CustomKingdomUtil::IF(vSpeed()[2]))
		{
			int num9 = Constants::motosRad2Axis(Constants::Motos_P_vecGetRad(v, vSpeed()));
			num9 = (num9 + 4) & 7;
			if (num9 > 4)
			{
				num9 = 8 - num9;
			}
			num7 *= Constants::get_g_AxisAttack()[num9 + 1];
		}
		else
		{
			num7 *= Constants::get_g_AxisAttack()[0];
		}
		ExecReflectSub(v, num7, num8);
	}
	if (cTaskChar5 != nullptr && CustomKingdomUtil::IF(num6 & 1) && cTaskChar5->bTestFlag(2u))
	{
		cTaskCharPlayer* cTaskCharPlayer2 = static_cast<cTaskCharPlayer*>(cTaskChar5);
		if (KingdomGlobal::g_pMotosGame->bMultiPlay())
		{
			if (!CustomKingdomUtil::IF(nPiyoTimer()))
			{
				nPiyoTimer() = Constants::get_g_ChargeLevel2PiyoTime()[cTaskCharPlayer2->nChargePowLevelOLD()];
			}
		}
		else
		{
			nPiyoTimer() = Constants::get_g_ChargeLevel2PiyoTime()[cTaskCharPlayer2->nChargePowLevelOLD()];
		}
	}
	RegistRefHistory(pPacket->m_nRefID, (int)Constants::GAME_COUNTER_FROMBYTE(pPacket->m_nCounter), flag, num5, pow, charge);
}

// DOL 0x801e5db8 [audited: per AUDIT_G03_G07 row]
void cTaskChar::ExecReflectSub(Vector4 pVec, float hit_pow, float charge_pow)
{
	Vector4 _out;
	Constants::VEC3_MUL(_out, pVec, charge_pow);
	::Constants::jetGeoVecCopy(vHSpeed(), _out);
	::Constants::jetGeoVecUnit(vSpeed());
	int num = (int)Mathf::Floor((float)nHereticVal() * hit_pow);
	if (!bTestFlag(8u) && num < 2)
	{
		num = 2;
	}
	SetHeresy((uint)num);
}

int cTaskChar::SendMessage(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (!ModeFunc(nMes, pListC, pScene))
	{
		return 0;
	}
	return 1;
}

// DOL 0x801e5ec0 [audited]
int cTaskChar::ExecModeFunc(cJetTask* pListC, cJetTask* pScene)
{
	if (m_nMode != m_nNextMode)
	{
		if (m_nMode != 0)
		{
			SendMessage(3, pListC, pScene);
		}
		m_nOldMode = m_nMode;
		m_nMode = m_nNextMode;
		if (m_nMode != 0)
		{
			SendMessage(2, pListC, pScene);
		}
	}
	return SendMessage(1, pListC, pScene);
}

// DOL 0x801e5f8c [audited]
bool cTaskChar::AdjustSpeedByHitAll(Vector4& pOutSpeed, Vector4 pTrans, Vector4 pSpeed)
{
	::Constants::jetGeoVecCopy(pOutSpeed, pSpeed);
	for (int i = 0; i < 36; i++)
	{
		if (i != (int)GrobalID())
		{
			cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)i);
			if (cTaskChar2 != nullptr && !cTaskChar2->bTestFlag(278624u))
			{
				AdjustSpeedByHit(cTaskChar2, pOutSpeed, pTrans, pOutSpeed);
			}
		}
	}
	return false;
}

bool cTaskChar::AdjustSpeedByHit(cTaskChar* pChar, Vector4& pOutSpeed, Vector4 pTrans, Vector4 pSpeed)
{
	pOutSpeed = Vector4();
	Constants::MOTOS_CAUTION("AdjustSpeedByHit");
	bool result = false;
	Vector4 pOut = Vector4();
	if (!Constants::IS_MYCHAR(this) || (bTestFlag(2u) && pChar->bTestFlag(16u)))
	{
		return false;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pSpeed);
	Vector4 _out2;
	if ((CustomKingdomUtil::IF(v[0]) || CustomKingdomUtil::IF(v[1]) || CustomKingdomUtil::IF(v[2])) && RefChar() != (int)pChar->GrobalID() && RefCharOLD() != (int)pChar->GrobalID() && KingdomGlobal::g_pMotosGame->pHitMan()->HitCheckSimple(this, pChar, {}, {}))
	{
		Vector4 v2;
		::Constants::jetGeoVecCopy(v2, pTrans);
		Vector4 _out;
		Constants::VEC3_ADD(_out, pTrans, v);
		Vector4 v3;
		::Constants::jetGeoVecCopy(v3, pChar->vMTrans());
		Vector4 _discard4;
		if (Constants::Motos_P_vecCrossCircleEx(pOut, _discard4, v2, _out, v3, (fHitSize() + pChar->fHitSize()) * 0.9f, false) != 0)
		{
			Constants::VEC3_SUB(_out2, pOut, pTrans);
			if (Constants::Motos_P_vecDot(_out2, v) >= 0.0f)
			{
				::Constants::jetGeoVecCopy(pOutSpeed, _out2);
			}
			else
			{
				::Constants::jetGeoVecUnit(pOutSpeed);
			}
			nHereticTimer() = 0u;
			result = true;
			::Constants::jetGeoVecCopy(v, pOutSpeed);
		}
	}
	if ((CustomKingdomUtil::IF(v[0]) || CustomKingdomUtil::IF(v[1]) || CustomKingdomUtil::IF(v[2])) && RefChar() != (int)pChar->GrobalID() && RefCharOLD() != (int)pChar->GrobalID() && pChar->ExHitCrossPoint(pOut, pTrans, v, fHitSize() * 0.9f))
	{
		Constants::VEC3_SUB(_out2, pOut, pTrans);
		if (Constants::Motos_P_vecDot(_out2, v) >= 0.0f)
		{
			::Constants::jetGeoVecCopy(pOutSpeed, v);
		}
		else
		{
			::Constants::jetGeoVecUnit(pOutSpeed);
		}
		nHereticTimer() = 0u;
		result = true;
	}
	return result;
}

bool cTaskChar::AdjustSpeedByHit(cTaskChar* pChar)
{
	bool result = false;
	Vector4 pOut = Vector4();
	Vector4 pSpeed = Vector4();
	Vector4 pSpeed2 = Vector4();
	if (Constants::IS_MYCHAR(this) && (!bTestFlag(2u) || !pChar->bTestFlag(16u)))
	{
		CalcSpeed(pSpeed);
		pChar->CalcSpeed(pSpeed2);
		Vector4 _out2;
		if ((CustomKingdomUtil::IF(pSpeed[0]) || CustomKingdomUtil::IF(pSpeed[1]) || CustomKingdomUtil::IF(pSpeed[2])) && RefChar() != (int)pChar->GrobalID() && RefCharOLD() != (int)pChar->GrobalID() && KingdomGlobal::g_pMotosGame->pHitMan()->HitCheckSimple(this, pChar, {}, {}))
		{
			Vector4 v;
			::Constants::jetGeoVecCopy(v, vMTrans());
			Vector4 _out;
			Constants::VEC3_ADD(_out, vMTrans(), pSpeed);
			Vector4 v2;
			::Constants::jetGeoVecCopy(v2, pChar->vMTrans());
			Vector4 _discard5;
			if (Constants::Motos_P_vecCrossCircleEx(pOut, _discard5, v, _out, v2, (fHitSize() + pChar->fHitSize()) * 0.9f, false) != 0)
			{
				Constants::VEC3_SUB(_out2, pOut, vMTrans());
				if (Constants::Motos_P_vecDot(_out2, pSpeed) >= 0.0f)
				{
					::Constants::jetGeoVecCopy(vSpeed(), _out2);
				}
				else
				{
					::Constants::jetGeoVecUnit(vSpeed());
				}
				nHereticTimer() = 0u;
				result = true;
				CalcSpeed(pSpeed);
				pChar->CalcSpeed(pSpeed2);
			}
		}
		if ((CustomKingdomUtil::IF(pSpeed[0]) || CustomKingdomUtil::IF(pSpeed[1]) || CustomKingdomUtil::IF(pSpeed[2])) && RefChar() != (int)pChar->GrobalID() && RefCharOLD() != (int)pChar->GrobalID() && pChar->ExHitCrossPoint(pOut, vMTrans(), pSpeed, fHitSize() * 0.9f))
		{
			Constants::VEC3_SUB(_out2, pOut, vMTrans());
			if (Constants::Motos_P_vecDot(_out2, pSpeed) >= 0.0f)
			{
				::Constants::jetGeoVecCopy(vSpeed(), _out2);
			}
			else
			{
				::Constants::jetGeoVecUnit(vSpeed());
			}
			nHereticTimer() = 0u;
			result = true;
		}
	}
	return result;
}

// DOL 0x801e6700 [audited: audited 44th] (audited 44th: matches; 1 = 180, 2 = 0, 4 = 270, 8 = 90, 5 = 225, 6 = 315, 9 = 135, 10 = 45 degrees)
void cTaskChar::Dir2Rotate(int dir)
{
	switch (dir)
	{
	case 8:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(90.0f), 0.0f);
		break;
	case 4:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(270.0f), 0.0f);
		break;
	case 1:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(180.0f), 0.0f);
		break;
	case 2:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(0.0f), 0.0f);
		break;
	case 5:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(225.0f), 0.0f);
		break;
	case 9:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(135.0f), 0.0f);
		break;
	case 6:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(315.0f), 0.0f);
		break;
	case 10:
		Constants::VEC3_SET(vRotate(), 0.0f, Constants::TSP_DEG2RAD(45.0f), 0.0f);
		break;
	case 3:
	case 7:
		break;
	}
}

void cTaskChar::Rotate2Dir(float rot)
{
	int num = Constants::motosRad2Axis(rot);
	switch (num)
	{
	case 0:
		num = 2;
		break;
	case 1:
		num = 10;
		break;
	case 2:
		num = 8;
		break;
	case 3:
		num = 9;
		break;
	case 4:
		num = 1;
		break;
	case 5:
		num = 5;
		break;
	case 6:
		num = 4;
		break;
	case 7:
		num = 6;
		break;
	}
	nModelDir() = (ushort)num;
}

// DOL 0x801e6868 [audited: per AUDIT_G03_G07 row]
bool cTaskChar::ExHitCrossPoint(Vector4& pOut, Vector4 pPos, Vector4 pSpeed, float HitSize)
{
	pOut = Vector4();
	Vector4 pOutCross = Vector4();
	if (nExHitNum() == 0)
	{
		return false;
	}
	Vector4 _out;
	Constants::VEC3_SUB(_out, pPos, vMTrans());
	Vector4 _out2;
	Constants::VEC3_ADD(_out2, _out, pSpeed);
	Constants::Motos_P_vecRotateVector(_out, _out, vRotate()[1]);
	Constants::Motos_P_vecRotateVector(_out2, _out2, vRotate()[1]);
	int num = 0;
	Constants::EX_HIT_PARAM* eX_HIT_PARAM;
	Constants::sMOTOS_P_RECT pRect = Constants::sMOTOS_P_RECT();
	while ((eX_HIT_PARAM = ExHitParam(num)) != nullptr)
	{
		Vector4 pOut2;
		Vector4 pOut3;
		if (eX_HIT_PARAM->m_nKind == 0)
		{
			pRect.x = eX_HIT_PARAM->m_square.m_vPoint0[0];
			pRect.y = eX_HIT_PARAM->m_square.m_vPoint0[2];
			pRect.w = eX_HIT_PARAM->m_square.m_vPoint1[0] - eX_HIT_PARAM->m_square.m_vPoint0[0];
			pRect.h = eX_HIT_PARAM->m_square.m_vPoint1[2] - eX_HIT_PARAM->m_square.m_vPoint0[2];
			pRect.x -= HitSize;
			pRect.y -= HitSize;
			pRect.w += HitSize * 2.0f;
			pRect.h += HitSize * 2.0f;
			if (Constants::Motos_P_vecCheckRect(pOutCross, pRect, _out, _out2))
			{
				Constants::Motos_P_vecRotateVector(pOutCross, pOutCross, 0.0f - vRotate()[1]);
				Constants::VEC3_ADD(pOut, pOutCross, vMTrans());
				return true;
			}
		}
		else if (eX_HIT_PARAM->m_nKind == 1 && Constants::Motos_P_vecCrossCircleEx(pOut2, pOut3, _out, _out2, eX_HIT_PARAM->m_circle.m_vOrigin, (fHitSize() + eX_HIT_PARAM->m_circle.m_fRange) * 0.9f, false) != 0)
		{
			Constants::Motos_P_vecRotateVector(pOut2, pOut2, 0.0f - vRotate()[1]);
			Constants::VEC3_ADD(pOut, pOut2, vMTrans());
		}
		num++;
	}
	return false;
}

Constants::EX_HIT_PARAM* cTaskChar::ExHitRegistRect(int n, Vector4 pOrg, float width, float depth, int height)
{
	Constants::EX_HIT_PARAM* eX_HIT_PARAM = ExHitParam(n);
	if (eX_HIT_PARAM != nullptr)
	{
		eX_HIT_PARAM->m_nFlag = (ushort)height;
		eX_HIT_PARAM->m_nKind = 0;
		::Constants::jetGeoVecCopy(eX_HIT_PARAM->m_square.m_vPoint0, pOrg);
		::Constants::jetGeoVecCopy(eX_HIT_PARAM->m_square.m_vPoint1, eX_HIT_PARAM->m_square.m_vPoint0);
		eX_HIT_PARAM->m_square.m_vPoint1[0] += width;
		eX_HIT_PARAM->m_square.m_vPoint1[2] += depth;
	}
	return eX_HIT_PARAM;
}

Constants::EX_HIT_PARAM* cTaskChar::ExHitRegistCircle(int n, Vector4 pOrg, float range, int height)
{
	Constants::EX_HIT_PARAM* eX_HIT_PARAM = ExHitParam(n);
	if (eX_HIT_PARAM != nullptr)
	{
		eX_HIT_PARAM->m_nFlag = (ushort)height;
		eX_HIT_PARAM->m_nKind = 1;
		::Constants::jetGeoVecCopy(eX_HIT_PARAM->m_circle.m_vOrigin, pOrg);
		eX_HIT_PARAM->m_circle.m_fRange = range;
	}
	return eX_HIT_PARAM;
}

void cTaskChar::ExHitAlloc(int num)
{
	m_nExHitNum = num;
	m_pExHitParam.assign(num, {});
	for (int i = 0; i < (int)m_pExHitParam.size(); i++)
	{
		m_pExHitParam[i] = new Constants::EX_HIT_PARAM();
	}
}

// DOL 0x801e6cd4 [audited: audited 44th] (audited 44th: matches; the C#'s AddGetItemCnt Steam statistic removed)
void cTaskChar::GetItem(int nGrobalID)
{
	if (!Constants::IS_MYCHAR(this))
	{
		return;
	}
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID);
	if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(16u))
	{
		if (KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[cTaskChar2->nKind()] < 99)
		{
			KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[cTaskChar2->nKind()]++;
		}
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE012_ITEM_GET);
		KingdomGlobal::MotosSys->bMotosCharDead((uint)nGrobalID) = true;
	}
}

void cTaskChar::CalcSpeed(Vector4& pSpeed)
{
	if (nHereticTimer() == 0)
	{
		::Constants::jetGeoVecCopy(pSpeed, vSpeed());
		return;
	}
	float num = HereticRate();
	float num2 = 1.0f - num;
	pSpeed[0] = vSpeed()[0] * num2 + vHSpeed()[0] * num;
	pSpeed[1] = vSpeed()[1] * num2 + vHSpeed()[1] * num;
	pSpeed[2] = vSpeed()[2] * num2 + vHSpeed()[2] * num;
}

void cTaskChar::SetMaterialAlpha(uint n, byte alpha)
{
	pModel(n);
}

bool cTaskChar::IsModelAnmReverse(uint nModel, uint nSlot, uint type)
{
	switch (type)
	{
	case 1u:
		if (pModel(nModel)->GetUpdateRateAnmChr(nSlot) < 0.0f)
		{
			return true;
		}
		return false;
	case 2u:
		if (pModel(nModel)->GetUpdateRateMatClr(nSlot) < 0.0f)
		{
			return true;
		}
		return false;
	default:
		CustomKingdomUtil::Assert("Failed! AnmType not Set!\n");
		return false;
	}
}

void cTaskChar::MortionReverse(uint nModel, uint nSlot, uint type)
{
	switch (type)
	{
	case 1u:
		pModel(nModel)->SetFrameAnmChr(nSlot, pModel(nModel)->GetFrameMaxAnmChr(nSlot));
		pModel(nModel)->SetUpdateRateAnmChr(nSlot, 0.0f - pModel(nModel)->GetUpdateRateAnmChr(nSlot));
		break;
	case 2u:
		pModel(nModel)->SetFrameMatClr(nSlot, pModel(nModel)->GetFrameMaxMatClr(nSlot));
		pModel(nModel)->SetUpdateRateMatClr(nSlot, 0.0f - pModel(nModel)->GetUpdateRateMatClr(nSlot));
		break;
	default:
		CustomKingdomUtil::Assert("Failed! AnmType not Set!\n");
		break;
	}
}

void cTaskChar::SetMortion(uint nModel, string name, uint nSlot, uint type)
{
	if (pModel(nModel) != nullptr && !name.empty())
	{
		pModel(nModel)->RegistAnm(nSlot, name);
		if (type != 0 && IsModelAnmReverse(nModel, nSlot, type))
		{
			MortionReverse(nModel, nSlot, type);
		}
	}
}

void cTaskChar::SetMortionReverse(uint nModel, string name, uint nSlot, uint type)
{
	if (pModel(nModel) != nullptr && !name.empty())
	{
		pModel(nModel)->RegistAnm(nSlot, name);
		MortionReverse(nModel, nSlot, type);
	}
}

}
