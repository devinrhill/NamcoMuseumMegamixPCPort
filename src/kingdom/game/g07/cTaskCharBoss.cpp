#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBomb.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

// TODO(unity): scEmissionColorPropertyID = Shader.PropertyToID("_EmissionColor");
const int cTaskCharBoss::scEmissionColorPropertyID = 0;
const Color cTaskCharBoss::scFlashColor = Color(1.0f, 1.0f, 1.0f, 0.0f);

cTaskCharBoss::cTaskCharBoss()
{
	Init();
}

bool cTaskCharBoss::TaikiAction()
{
	return false;
}

int& cTaskCharBoss::nHp()
{
	return m_nHp;
}

int& cTaskCharBoss::nMaxHp()
{
	return m_nMaxHp;
}

float& cTaskCharBoss::fChargeDamageRate()
{
	return m_fChargeDamageRate;
}

int& cTaskCharBoss::nScore()
{
	return m_nScore;
}

uint& cTaskCharBoss::nApearDemoTimer()
{
	return m_nApearDemoTimer;
}

float& cTaskCharBoss::fHitQuake()
{
	return m_fHitQuake;
}

int& cTaskCharBoss::nHitQuakeTime()
{
	return m_nHitQuakeTime;
}

int& cTaskCharBoss::nBombMax()
{
	return m_nBombMax;
}

int& cTaskCharBoss::nBombTime()
{
	return m_nBombTime;
}

cTaskCharEnemyBomb* cTaskCharBoss::pSpawnBomb(int n)
{
	return m_pSpawnBomb[32];
}

float& cTaskCharBoss::fFogNear()
{
	return near_;
}

float& cTaskCharBoss::fFogFar()
{
	return far_;
}

float& cTaskCharBoss::fCoreFogNear()
{
	return m_fCoreFogNear;
}

float& cTaskCharBoss::fCoreFogFar()
{
	return m_fCoreFogFar;
}

uint& cTaskCharBoss::nCoreFogColor()
{
	return m_nCoreFogColor;
}

int& cTaskCharBoss::nExplodeTimer()
{
	return m_nExplodeTimer;
}

void cTaskCharBoss::ReturnStep(ushort step)
{
	m_nReturnStep = step;
}

ushort cTaskCharBoss::ReturnStep()
{
	return m_nReturnStep;
}

int& cTaskCharBoss::nAction()
{
	return m_nAction;
}

int& cTaskCharBoss::nMovePoint()
{
	return m_nMovePoint;
}

void cTaskCharBoss::StepInit()
{
	Step() = (Step(1) = (Step(2) = (Step(3) = 0)));
}

bool cTaskCharBoss::IsApearDemoEnd()
{
	return m_bAppearDemoEnd;
}

bool& cTaskCharBoss::bApearDemoEnd()
{
	return m_bAppearDemoEnd;
}

void cTaskCharBoss::Init()
{
	m_bAppearDemoEnd = false;
	OnFlag(8u);
	OnFlag(4u);
	m_nApearDemoTimer = 0u;
	m_nBombMax = 0;
	m_nBombTime = 0;
	for (int i = 0; i < (int)std::size(m_pSpawnBomb); i++)
	{
		m_pSpawnBomb[i] = nullptr;
	}
	m_nApearDemoTimerTmp = 0u;
	m_nAction = 0;
	m_nReturnStep = 0;
	m_nMovePoint = 0;
	m_nHp = 0;
	m_fChargeDamageRate = 0.0f;
	m_nExplodeTimer = 0;
	m_fHitQuake = 0.0f;
	m_nHitQuakeTime = 0;
	KingdomGlobal::g_pMotosGame->pBoss() = this;
	GetFogData();
}

void cTaskCharBoss::Term()
{
	cTaskCharEnemy::Term();
}

bool cTaskCharBoss::Execute(cJetTask* pParam)
{
	m_nowFramePlaySEHitZako01 = false;
	m_nowFramePlaySEHitBoss01 = false;
	if (KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > nHp())
	{
		KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.1f, 20);
	}
	if (KingdomGlobal::MotosSys->nStageIdx() == 35)
	{
		if (nKind() == 26)
		{
			KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHPLast = (short)nHp();
		}
		else
		{
			KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP = (short)nHp();
		}
	}
	else
	{
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP = (short)nHp();
	}
	if (bTestFlag(4194304u))
	{
		bool flag = KingdomGlobal::g_pMotosGame->IsGameStart();
		KingdomGlobal::g_pMotosGame->IsGameStart() = true;
		bool result = cTaskChar::Execute(pParam);
		KingdomGlobal::g_pMotosGame->IsGameStart() = flag;
		return result;
	}
	if (m_nHitQuakeTime > 0)
	{
		m_fHitQuake = (float)Random::Range(0, 10) * 0.02f;
		m_nHitQuakeTime--;
	}
	else
	{
		m_fHitQuake = 0.0f;
	}
	return cTaskChar::Execute(pParam);
}

void cTaskCharBoss::PresetEnv()
{
}

void cTaskCharBoss::RegistPacket()
{
}

void cTaskCharBoss::SyncPacket()
{
}

void cTaskCharBoss::CreateBomb()
{
	for (int i = 0; i < m_nBombMax; i++)
	{
		Vector4 _discard1;
		Constants::VEC3_SET(_discard1, 0.0f, 0.0f, 50.0f);
		if (m_pSpawnBomb[i] == nullptr)
		{
			m_pSpawnBomb[i] = new cTaskCharEnemyBomb();
			m_pSpawnBomb[i]->Create(0u);
		}
		m_pSpawnBomb[i]->ChangeTaiki();
		m_pSpawnBomb[i]->InitParam(Constants::get_g_MoEnemyParam()[25]);
		::Constants::jetGeoVecUnit(m_pSpawnBomb[i]->vRotate());
		::Constants::jetGeoVecUnit(m_pSpawnBomb[i]->vScale());
		m_pSpawnBomb[i]->vScale()[0] = 1.0f;
		m_pSpawnBomb[i]->vScale()[1] = 1.0f;
		m_pSpawnBomb[i]->vScale()[2] = 1.0f;
		m_pSpawnBomb[i]->nKind() = 25u;
		m_pSpawnBomb[i]->nChipColor() = m_pSpawnBomb[i]->Kind2ChipColor(25);
		ConnectChild(m_pSpawnBomb[i]);
	}
}

cTaskCharEnemyBomb* cTaskCharBoss::SpawnBombRot(float rot, float range, float fsp, float nohit_range, float offset)
{
	Vector4 v;
	Constants::tspVector3RotY(v, Constants::VEC3_GET(0.0f, 0.0f, 1.0f), rot);
	cTaskCharEnemyBomb* taikiBomb = GetTaikiBomb();
	if (taikiBomb != nullptr)
	{
		taikiBomb->ChangeTaiki();
		taikiBomb->SetFrame(0u, 0.0f);
		taikiBomb->OnFlag(32u);
		fSlopePower() = 0.0f;
		Vector4 _out;
		Constants::VEC3_MUL(_out, v, offset);
		::Constants::jetGeoVecCopy(taikiBomb->vMTrans(), vMTrans());
		Constants::VEC3_ADD(taikiBomb->vMTrans(), taikiBomb->vMTrans(), _out);
		::Constants::jetGeoVecCopy(taikiBomb->vSpawnPos(), vMTrans());
		taikiBomb->nInputDir() = 0;
		taikiBomb->fShootRange() = range;
		taikiBomb->fNoHitRange() = nohit_range;
		Vector4 _out2;
		Constants::VEC3_MUL(_out2, v, range);
		Constants::VEC3_ADD(taikiBomb->vFTarget(), vMTrans(), _out2);
		Constants::VEC3_MUL(taikiBomb->vSpeed(), v, fsp);
		taikiBomb->fMaxSpeed() = fsp;
		taikiBomb->nLTimer() = m_nBombTime;
	}
	return taikiBomb;
}

cTaskCharEnemyBomb* cTaskCharBoss::SpawnBomb(int dir, float range, float fsp, float nohit_range, float offset)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(dir));
	cTaskCharEnemyBomb* taikiBomb = GetTaikiBomb();
	if (taikiBomb != nullptr)
	{
		taikiBomb->ChangeTaiki();
		taikiBomb->SetFrame(0u, 0.0f);
		taikiBomb->OnFlag(32u);
		fSlopePower() = 0.0f;
		Vector4 _discard2;
		Constants::VEC3_MUL(_discard2, v, offset);
		::Constants::jetGeoVecCopy(taikiBomb->vMTrans(), vMTrans());
		Constants::VEC3_ADD(taikiBomb->vMTrans(), taikiBomb->vMTrans(), v);
		::Constants::jetGeoVecCopy(taikiBomb->vSpawnPos(), vMTrans());
		taikiBomb->nInputDir() = (ushort)dir;
		taikiBomb->fShootRange() = range;
		taikiBomb->fNoHitRange() = nohit_range;
		Vector4 _out2;
		Constants::VEC3_MUL(_out2, v, range);
		Constants::VEC3_ADD(taikiBomb->vFTarget(), vMTrans(), _out2);
		Constants::VEC3_MUL(taikiBomb->vSpeed(), v, fsp);
		taikiBomb->fMaxSpeed() = fsp;
		taikiBomb->nLTimer() = m_nBombTime;
	}
	return taikiBomb;
}

cTaskCharEnemyBomb* cTaskCharBoss::GetTaikiBomb()
{
	for (int i = 0; i < m_nBombMax; i++)
	{
		if (m_pSpawnBomb[i] != nullptr && m_pSpawnBomb[i]->bTestFlag(16384u) && m_pSpawnBomb[i]->vMTrans()[2] >= 50.0f)
		{
			return m_pSpawnBomb[i];
		}
	}
	return nullptr;
}

void cTaskCharBoss::BombAllExplode()
{
	for (int i = 0; i < m_nBombMax; i++)
	{
		if (m_pSpawnBomb[i] != nullptr && !m_pSpawnBomb[i]->bTestFlag(16384u) && !m_pSpawnBomb[i]->IsFall())
		{
			Constants::SpawnEffect(19, m_pSpawnBomb[i]->vMTrans());
			m_pSpawnBomb[i]->ChangeTaiki();
		}
	}
}

void cTaskCharBoss::SetShadow(uint kind)
{
}

void cTaskCharBoss::SpawnBossColEffect(cTaskChar* pChar, int kind, int chip)
{
	if (pChar != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pTaskEffect()->ColPos(pChar, this));
		if (kind == -1)
		{
			Constants::SpawnEffect(2, v);
		}
		else
		{
			Constants::SpawnEffect(kind, v);
		}
		int num = ((chip != -1) ? chip : nChipColor());
		Constants::SpawnEffect(num, v);
		Constants::SpawnEffect(num, v);
		Constants::SpawnEffect(num, v);
		if (!m_nowFramePlaySEHitZako01)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE001_HIT_ZAKO01);
			m_nowFramePlaySEHitZako01 = true;
		}
	}
}

void cTaskCharBoss::SpawnBossColEffect_Core(cTaskChar* pChar, int kind, int chip)
{
	if (pChar != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pTaskEffect()->ColPos(pChar, this));
		if (kind == -1)
		{
			Constants::SpawnEffect(3, v);
		}
		else
		{
			Constants::SpawnEffect(kind, v);
		}
		int num = ((chip != -1) ? chip : 7);
		Constants::SpawnEffect(num, v);
		Constants::SpawnEffect(num, v);
		Constants::SpawnEffect(num, v);
		if (!m_nowFramePlaySEHitBoss01)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE004_HIT_BOSS01);
			m_nowFramePlaySEHitBoss01 = true;
		}
	}
}

void cTaskCharBoss::SpawnAppearEffect(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int nKind)
{
	Vector4 v = Vector4();
	Vector4 v2 = Vector4::zero;
	int num = 0;
	int num2 = (int)(lx / space) + 1;
	int num3 = (int)(lz / space) + 1;
	for (int i = 0; i < num2; i++)
	{
		for (int j = 0; j < num3; j++)
		{
			v[0] = porg[0] + space * (float)i + 0.1f * (float)(Random::Range(0, 10) - 5);
			v[1] = 0.0f;
			v[2] = porg[2] + space * (float)j + 0.1f * (float)(Random::Range(0, 10) - 5);
			::Constants::jetGeoVecUnit(v2);
			Constants::VEC3_SUB(v2, v2, v);
			num = (int)Mathf::Floor(::Constants::jetGeoVecLength(v2) * 1.5f);
			(void)((double)::Constants::jetGeoVecLength(v2));
			(void)(2.0);
			Constants::tspVector3RotZYX(v, v, pchar->vRotate());
			Constants::VEC3_ADD(v, v, pchar->vMTrans());
			if (nKind == -1)
			{
				Constants::SpawnEffect(24, v, this, num);
			}
			else
			{
				Constants::SpawnEffect(nKind, v, this);
			}
		}
	}
}

void cTaskCharBoss::SpawnAppearEffectCircle(cTaskChar* pchar, Vector4 porg, float lr, float space)
{
	Vector4 v = Vector4();
	int num = (int)(lr * 2.0f / space) + 1;
	int num2 = (int)(lr * 2.0f / space) + 1;
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			v[0] = porg[0] - lr + space * (float)i + 0.1f * (float)(Random::Range(0, 10) - 5);
			v[1] = 0.0f;
			v[2] = porg[2] - lr + space * (float)j + 0.1f * (float)(Random::Range(0, 10) - 5);
			Vector4 _out;
			Constants::VEC3_SUB(_out, porg, v);
			if (::Constants::jetGeoVecLength(_out) <= lr)
			{
				Constants::tspVector3RotZYX(v, v, pchar->vRotate());
				Constants::VEC3_ADD(v, v, pchar->vMTrans());
				Constants::SpawnEffect(24, v, this, Random::Range(0, 15));
			}
		}
	}
}

void cTaskCharBoss::SpawnExplode(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float lz, float space, int delay)
{
	Vector4 v = Vector4();
	float num = 1.5f;
	int num2 = (int)(lx / space) + 1;
	int num3 = (int)(lz / space) + 1;
	for (int i = 0; i < num2; i++)
	{
		for (int j = 0; j < num3; j++)
		{
			v[0] = porg[0] + space * (float)i + 0.15f * (float)(Random::Range(0, 10) - 5);
			v[1] = porg[1] + num * 0.15f * (float)(Random::Range(0, 10) - 5);
			v[2] = porg[2] + space * (float)j + 0.15f * (float)(Random::Range(0, 10) - 5);
			::Constants::jetGeoVecRotZYX(v, v, rot);
			Constants::VEC3_ADD(v, v, trans);
			Constants::SpawnEffect(22, v, nullptr, Random::Range(0, 20) + delay, 0.3f);
		}
	}
}

void cTaskCharBoss::SpawnExplode(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int delay)
{
	SpawnExplode(pchar->vMTrans(), pchar->vRotate(), porg, lx, lz, space, delay);
}

void cTaskCharBoss::SpawnExplodeRandom(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float lz, float space, int rand_span, int delay)
{
	Vector4 v = Vector4();
	float num = 1.5f;
	int num2 = (int)(lx / space) + 1;
	int num3 = (int)(lz / space) + 1;
	for (int i = 0; i < num2; i++)
	{
		for (int j = 0; j < num3; j++)
		{
			v[0] = porg[0] + space * (float)i + 0.1f * (float)(Random::Range(0, 10) - 5);
			v[1] = porg[1] + num * 0.1f * (float)(Random::Range(0, 10) - 5);
			v[2] = porg[2] + space * (float)j + 0.1f * (float)(Random::Range(0, 10) - 5);
			Constants::tspVector3RotZYX(v, v, rot);
			Constants::VEC3_ADD(v, v, trans);
			Constants::SpawnEffect(22, v, nullptr, Random::Range(0, rand_span) + delay);
		}
	}
}

void cTaskCharBoss::SpawnExplodeRandom(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int rand_span, int delay)
{
	SpawnExplodeRandom(pchar->vMTrans(), pchar->vRotate(), porg, lx, lz, space, rand_span, delay);
}

void cTaskCharBoss::SpawnExplode2(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float ly, float space, int delay)
{
	Vector4 v = Vector4();
	int num = (int)(lx / space) + 1;
	int num2 = (int)(ly / space) + 1;
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			v[0] = porg[0] + space * (float)i + 0.1f * (float)(Random::Range(0, 10) - 5);
			v[1] = porg[1] + space * (float)j + 0.1f * (float)(Random::Range(0, 10) - 5);
			v[2] = porg[2] + 0.1f * (float)(Random::Range(0, 10) - 5);
			Constants::tspVector3RotZYX(v, v, rot);
			Constants::VEC3_ADD(v, v, trans);
			Constants::SpawnEffect(22, v, nullptr, Random::Range(0, 20) + delay);
		}
	}
}

void cTaskCharBoss::SpawnExplode2(cTaskChar* pchar, Vector4 porg, float lx, float ly, float space, int delay)
{
	SpawnExplode2(pchar->vMTrans(), pchar->vRotate(), porg, lx, ly, space, delay);
}

void cTaskCharBoss::SpawnExplode2Random(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float ly, float space, int rand_span, int delay)
{
	Vector4 v = Vector4();
	int num = (int)(lx / space) + 1;
	int num2 = (int)(ly / space) + 1;
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			v[0] = porg[0] + space * (float)i + 0.02f * (float)(Random::Range(0, 10) - 5);
			v[1] = porg[1] + space * (float)j + 0.02f * (float)(Random::Range(0, 10) - 5);
			v[2] = porg[2] + 0.02f * (float)(Random::Range(0, 10) - 5);
			Constants::tspVector3RotZYX(v, v, rot);
			Constants::VEC3_ADD(v, v, trans);
			Constants::SpawnEffect(22, v, nullptr, Random::Range(0, rand_span) + delay);
		}
	}
}

void cTaskCharBoss::SpawnExplode2Random(cTaskChar* pchar, Vector4 porg, float lx, float ly, float space, int rand_span, int delay)
{
	SpawnExplode2Random(pchar->vMTrans(), pchar->vRotate(), porg, lx, ly, space, rand_span, delay);
}

float cTaskCharBoss::IsAdjustRotate(float rot, float arot, float add)
{
	if (rot > (float)std::numbers::pi * 2.0f || rot < 0.0f)
	{
		rot -= (float)std::numbers::pi * 2.0f * Mathf::Floor(rot / ((float)std::numbers::pi * 2.0f));
	}
	if (rot == arot || (rot < arot && rot + add >= arot) || (rot > arot && rot + add <= arot) || (rot > arot && rot + add - (float)std::numbers::pi * 2.0f >= arot) || (rot < arot && rot + add + (float)std::numbers::pi * 2.0f <= arot))
	{
		return arot;
	}
	return -1.0f;
}

int cTaskCharBoss::HitDamage(cTaskChar* pChar, const int* pHitDamageTbl)
{
	int num = 0;
	if (pChar->bTestFlag(2u))
	{
		int num2 = (static_cast<cTaskCharPlayer*>(pChar))->nPowerLevel();
		if (num2 > 5)
		{
			num2 = 5;
		}
		else if (num2 < 0)
		{
			num2 = 0;
		}
		num = pHitDamageTbl[num2];
		if (pChar->bTestFlagOLD(4096u) && fChargeDamageRate() > 0.0f)
		{
			num = (int)((float)num * fChargeDamageRate());
		}
		SetHitQuake(10);
	}
	else
	{
		num = pHitDamageTbl[0];
		SetHitQuake(10);
	}
	return num;
}

void cTaskCharBoss::ExplodeSE(uint nRandMin, uint nRandMax)
{
	if (CustomKingdomUtil::IF(nExplodeTimer()) && (long)Constants::get_GAME_COUNTER() % (long)nExplodeTimer() == 0L)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL);
	}
	nExplodeTimer() = (int)((float)nRandMin + Random::Range(0.0f, nRandMax - nRandMin));
}

bool cTaskCharBoss::BossFadeAnitaDraw(cJetTask* pTask, cJetTask* pList, cJetTask* pScene)
{
	return true;
}

void cTaskCharBoss::SetHitQuake(int time)
{
	m_nHitQuakeTime = time;
}

void cTaskCharBoss::GetFogData()
{
}

void cTaskCharBoss::InitFogData()
{
}

void cTaskCharBoss::SetFlash(int modelIdx, float rate)
{
	if (modelIdx >= 0 && modelIdx < (int)m_flashMatLists.size() && modelIdx < (int)m_baseMatColorLists.size())
	{
		for (int i = 0; i < (int)m_flashMatLists[modelIdx].size(); i++)
		{
			Color value = Color::Lerp(m_baseMatColorLists[modelIdx][i], scFlashColor, rate);
			// TODO(unity): m_flashMatLists[modelIdx][i].SetColor(scEmissionColorPropertyID, value);
		}
	}
}

}
