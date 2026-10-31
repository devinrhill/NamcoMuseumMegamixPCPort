#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include <cstring>
#include "kingdom/MslRand.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cTaskConsoleMan.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharBEye.hpp"
#include "kingdom/game/g03/cTaskCharGolvis01.hpp"
#include "kingdom/game/g03/cTaskCharGolvis02.hpp"
#include "kingdom/game/g03/cTaskCharGolvis03.hpp"
#include "kingdom/game/g03/cTaskCharGolvis04.hpp"
#include "kingdom/game/g03/cTaskCharGolvis05.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskLandAttack.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

// DOL table 0x8043eed8 {id, name}: BOOT_* = 20..37, LAND_* = 100..125 (43rd session: the C# numbered them 15..58)
const cTaskCharGolvis::GOLVIS_NAME2ID* cTaskCharGolvis::s_aAttackName[59] = {[]() { cTaskCharGolvis::GOLVIS_NAME2ID* _o1 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o1->m_nID = 0; _o1->m_pName = "NONE"; return _o1; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o2 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o2->m_nID = 1; _o2->m_pName = "GUITAR_V_S"; _o2->m_num = 1; return _o2; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o3 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o3->m_nID = 2; _o3->m_pName = "GUITAR_V_M"; return _o3; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o4 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o4->m_nID = 3; _o4->m_pName = "GUITAR_V_F"; return _o4; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o5 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o5->m_nID = 4; _o5->m_pName = "GUITAR_H_S"; return _o5; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o6 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o6->m_nID = 5; _o6->m_pName = "GUITAR_H_M"; return _o6; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o7 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o7->m_nID = 6; _o7->m_pName = "GUITAR_H_F"; return _o7; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o8 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o8->m_nID = 7; _o8->m_pName = "BOMB"; return _o8; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o9 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o9->m_nID = 8; _o9->m_pName = "CYCLONE_INHALE"; return _o9; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o10 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o10->m_nID = 9; _o10->m_pName = "CYCLONE_DISGORGE"; return _o10; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o11 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o11->m_nID = 10; _o11->m_pName = "HEADING_S"; return _o11; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o12 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o12->m_nID = 11; _o12->m_pName = "HEADING_M"; return _o12; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o13 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o13->m_nID = 12; _o13->m_pName = "HEADING_F"; return _o13; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o14 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o14->m_nID = 13; _o14->m_pName = "ELEC"; return _o14; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o15 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o15->m_nID = 14; _o15->m_pName = "FAINT"; return _o15; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o16 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o16->m_nID = 20; _o16->m_pName = "BOOT_0"; _o16->m_num = 20; return _o16; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o17 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o17->m_nID = 21; _o17->m_pName = "BOOT_1"; return _o17; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o18 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o18->m_nID = 22; _o18->m_pName = "BOOT_2"; return _o18; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o19 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o19->m_nID = 23; _o19->m_pName = "BOOT_3"; return _o19; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o20 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o20->m_nID = 24; _o20->m_pName = "BOOT_4"; return _o20; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o21 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o21->m_nID = 25; _o21->m_pName = "BOOT_5"; return _o21; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o22 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o22->m_nID = 26; _o22->m_pName = "BOOT_6"; return _o22; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o23 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o23->m_nID = 27; _o23->m_pName = "BOOT_7"; return _o23; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o24 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o24->m_nID = 28; _o24->m_pName = "BOOT_8"; return _o24; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o25 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o25->m_nID = 29; _o25->m_pName = "BOOT_9"; return _o25; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o26 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o26->m_nID = 30; _o26->m_pName = "BOOT_0_1"; return _o26; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o27 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o27->m_nID = 31; _o27->m_pName = "BOOT_2_3"; return _o27; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o28 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o28->m_nID = 32; _o28->m_pName = "BOOT_4_5"; return _o28; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o29 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o29->m_nID = 33; _o29->m_pName = "BOOT_6_7"; return _o29; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o30 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o30->m_nID = 34; _o30->m_pName = "BOOT_8_9"; return _o30; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o31 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o31->m_nID = 35; _o31->m_pName = "BOOT_0_1_2"; return _o31; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o32 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o32->m_nID = 36; _o32->m_pName = "BOOT_3_4_5"; return _o32; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o33 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o33->m_nID = 37; _o33->m_pName = "BOOT_6_7_8"; return _o33; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o34 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o34->m_nID = 100; _o34->m_pName = "LAND_00"; _o34->m_num = 100; return _o34; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o35 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o35->m_nID = 101; _o35->m_pName = "LAND_01"; return _o35; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o36 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o36->m_nID = 102; _o36->m_pName = "LAND_02"; return _o36; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o37 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o37->m_nID = 103; _o37->m_pName = "LAND_03"; return _o37; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o38 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o38->m_nID = 104; _o38->m_pName = "LAND_04"; return _o38; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o39 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o39->m_nID = 105; _o39->m_pName = "LAND_05"; return _o39; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o40 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o40->m_nID = 106; _o40->m_pName = "LAND_06"; return _o40; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o41 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o41->m_nID = 107; _o41->m_pName = "LAND_07"; return _o41; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o42 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o42->m_nID = 108; _o42->m_pName = "LAND_08"; return _o42; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o43 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o43->m_nID = 109; _o43->m_pName = "LAND_09"; return _o43; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o44 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o44->m_nID = 110; _o44->m_pName = "LAND_10"; return _o44; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o45 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o45->m_nID = 111; _o45->m_pName = "LAND_11"; return _o45; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o46 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o46->m_nID = 112; _o46->m_pName = "LAND_12"; return _o46; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o47 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o47->m_nID = 113; _o47->m_pName = "LAND_13"; return _o47; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o48 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o48->m_nID = 114; _o48->m_pName = "LAND_14"; return _o48; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o49 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o49->m_nID = 115; _o49->m_pName = "LAND_15"; return _o49; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o50 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o50->m_nID = 116; _o50->m_pName = "LAND_16"; return _o50; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o51 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o51->m_nID = 117; _o51->m_pName = "LAND_17"; return _o51; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o52 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o52->m_nID = 118; _o52->m_pName = "LAND_18"; return _o52; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o53 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o53->m_nID = 119; _o53->m_pName = "LAND_19"; return _o53; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o54 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o54->m_nID = 120; _o54->m_pName = "LAND_20"; return _o54; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o55 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o55->m_nID = 121; _o55->m_pName = "LAND_21"; return _o55; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o56 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o56->m_nID = 122; _o56->m_pName = "LAND_22"; return _o56; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o57 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o57->m_nID = 123; _o57->m_pName = "LAND_23"; return _o57; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o58 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o58->m_nID = 124; _o58->m_pName = "LAND_24"; return _o58; }(), []() { cTaskCharGolvis::GOLVIS_NAME2ID* _o59 = new cTaskCharGolvis::GOLVIS_NAME2ID(); _o59->m_nID = 125; _o59->m_pName = "LAND_25"; return _o59; }()};

cTaskCharGolvis::sBossDeathDemo::sBossDeathDemo()
{
	nProc = 0u;
	bChange = true;
	bUseDeathDemo = false;
}

// DOL 0x801184f8 [audited]
Vector4 cTaskCharGolvis::CalcButtCursorTrans(Vector4 v)
{
	return v;
}

bool& cTaskCharGolvis::bUseDeathDemo()
{
	return m_sDeathDemo->bUseDeathDemo;
}

uint cTaskCharGolvis::GetDeathProc()
{
	return m_sDeathDemo->nProc;
}

void cTaskCharGolvis::ChangeDeathProc(uint nProc)
{
	m_sDeathDemo->nProc = nProc;
	m_sDeathDemo->bChange = true;
}

bool cTaskCharGolvis::IsDeathProcFirstCall()
{
	if (m_sDeathDemo->bChange)
	{
		m_sDeathDemo->bChange = false;
		return true;
	}
	return false;
}

// DOL 0x801192ac [audited]
void cTaskCharGolvis::MonsterFrameMove_PreProcess()
{
}

// DOL 0x801184f4 [audited]
void cTaskCharGolvis::MonsterFrameMove_Free()
{
}

// DOL 0x801192b0 [audited]
void cTaskCharGolvis::MonsterFrameMove_Escape()
{
}

// DOL 0x801192c0 [audited]
bool cTaskCharGolvis::MonsterFrameMove_Death()
{
	return false;
}

// DOL 0x80116d34 [audited]
bool cTaskCharGolvis::IsForceAttack()
{
	return false;
}

// DOL 0x80119b0c [audited]
cTaskLandAttack* cTaskCharGolvis::pAttackLand()
{
	return m_pAttackLand;
}

// DOL 0x80119b08 [audited]
void cTaskCharGolvis::InitStateFirst()
{
}

// DOL 0x80119b00 [audited]
Vector4 cTaskCharGolvis::vFirstTrans()
{
	return m_vFirstTrans;
}

// DOL 0x80119af8 [audited]
Vector4 cTaskCharGolvis::vFirstRotate()
{
	return m_vFirstRotate;
}

// DOL 0x801192b4 [audited]
void cTaskCharGolvis::InitStateStart()
{
	m_iAIMode = 2;
}

// DOL 0x801160f4 [audited: index formula (eaten*3+level)*8+n checked]
Constants::gmSGolvisAttack* cTaskCharGolvis::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	CustomKingdomUtil::Assert((nEatNum * 3 + nIjikeLevel) * 8 + nNum < 72, "over tbl");
	return m_pAttackTBL[(nEatNum * 3 + nIjikeLevel) * 8 + nNum];
}

// DOL 0x80119298 [audited]
bool cTaskCharGolvis::IsBossDeath()
{
	return m_iEatenNum == 3;
}

bool cTaskCharGolvis::IsBijikeMsgCome()
{
	return m_bIjikeMsgCome;
}

// DOL 0x800b8050 [audited]
void cTaskCharGolvis::IjikeStart()
{
	m_bIjikeMsgCome = true;
}

uint cTaskCharGolvis::nLife()
{
	return (uint)(3 - m_iEatenNum);
}

int cTaskCharGolvis::GetBijikeLevel()
{
	return m_iIzikeLevel;
}

int cTaskCharGolvis::GetSubState()
{
	return m_iSubState;
}

int cTaskCharGolvis::GetAIMode()
{
	return m_iAIMode;
}

int cTaskCharGolvis::GetCount(uint val)
{
	CustomKingdomUtil::Assert(val < 3, "error");
	return m_iCount[val];
}

cTaskCharGolvis::cTaskCharGolvis(cJetTask* pParent)
	: m_sDeathDemo(new cTaskCharGolvis::sBossDeathDemo())
{
	if (pParent != nullptr)
	{
		pParent->ConnectChild(this);
	}
	SetName("cTaskCharGolvis");
	Init();
}

// DOL 0x80113f78 [audited: CSV loop, hit chars (0.9 / 0.8, offset 3.0) vs decomp; node callback FUN_801153ac and sound actor not ported]
void cTaskCharGolvis::Init()
{
	m_hPtkStorm = new cKingdomPtkHnd();
	m_hPtkStorm2 = new cKingdomPtkHnd();
	m_hPtkConf = new cKingdomPtkHnd();
	m_hPtkSweat = new cKingdomPtkHnd();
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(vRotate(), 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
	SetModel();
	m_nAnmNew = 0u;
	m_nAnmNow = 34u;
	m_nAnmOld = 34u;
	m_nAnmChgCount = 0u;
	m_nAnmChgTime = 0u;
	m_nAnmSw = 0u;
	nKind() = cTaskChar::eKIND::eKIND_BOSS;
	m_fWindGuiterLength = 1.0f;
	m_bIjikeMsgCome = false;
	pModel(nType())->Visible();   // DOL FUN_8009a454(+0x1d0) + FUN_8001a1f4
	m_pBijikeEyeObj = cTaskCharBEye::Create(this);
	int num = -1;
	int num2 = 0;
	cKingdomCSV* cKingdomCSV2 = new cKingdomCSV();
	cKingdomCSV2->RegistData("BIN_CSV_BOSS_ATTACK");
	std::fill(std::begin(m_pAttackTBL), std::end(m_pAttackTBL), nullptr);
	for (int i = 0; i < (int)std::size(m_pAttackTBL); i++)
	{
		m_pAttackTBL[i] = new Constants::gmSGolvisAttack();
	}
	int num3 = cKingdomCSV2->nRow();
	for (int j = 1; j < num3; j++)
	{
		string _out;
		if (cKingdomCSV2->Str(j, 0, _out, 128u) && CustomKingdomUtil::strncmp(_out, "LIFE", 4) == 0)
		{
			num++;
			num2 = 0;
		}
		else if (num != -1)
		{
			Constants::gmSGolvisAttack* gmSGolvisAttack = m_pAttackTBL[num * 8 + num2];
			if (cKingdomCSV2->Str(j, 0, _out, 128u))
			{
				gmSGolvisAttack->uiKind = (int)AttackName2ID(_out);
			}
			float _out2;
			if (cKingdomCSV2->F32(j, 1, _out2))
			{
				gmSGolvisAttack->fRangeMin = _out2;
			}
			if (cKingdomCSV2->F32(j, 2, _out2))
			{
				gmSGolvisAttack->fRangeMax = _out2;
			}
			int _out3;
			if (cKingdomCSV2->S32(j, 3, _out3))
			{
				gmSGolvisAttack->iTimeMin = (short)_out3;
			}
			if (cKingdomCSV2->S32(j, 4, _out3))
			{
				gmSGolvisAttack->iTimeMax = (short)_out3;
			}
			num2++;
		}
	}
	cHitChar* cHitChar2 = new cHitChar(this, 1);
	cHitChar2->SetRange(0.9f);
	pvecHitChar().push_back(cHitChar2);
	cHitChar* cHitChar3 = new cHitChar(this, 1, 2);
	cHitChar3->SetRange(0.8f);
	cHitChar3->SetOfsPos(0.0f, 0.0f, 3.0f);
	pvecHitChar().push_back(cHitChar3);
	// (the C# looked up a Unity Transform for ATAMA_jnt here; the DOL takes the head matrix in the node callback FUN_801153ac, Draw reads
	// the node directly. The DOL then creates the sound actor +0x860 = FUN_800ba17c: the port's SE layer has no actors, engine item E1.)
}

// DOL 0x80114ec8 [audited: checked (the DOL does not terminate the bijike eye here)]
void cTaskCharGolvis::Term()
{
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE026_KAZE);
	if (m_pBijikeEyeObj != nullptr)
	{
		m_pBijikeEyeObj->TermOnce();
	}
	cTaskCharEnemy::Term();
}

// DOL 0x80114f34 [audited: flow checked; EraseConf's model flag FUN_802fac10(resmdl+0x58, 2) not ported]
bool cTaskCharGolvis::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		Constants::PACN_APP()->pTaskCollision()->PushBackChar(this);
	}
	if (m_iSubState != 14 || nType() != 0)
	{
		EraseConfEffect();
	}
	if (!m_hPtkConf->IsValid() && nullptr != m_hSndPiyori)
	{
		SoundManager::get_SE()->Stop(m_hSndPiyori);
	}
	if (pCurrentModel()->vTrans()[1] < -30.0f)
	{
		ChangeToRevive();
	}
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss() && !Constants::IS_NAKAJIMA_PAUSE(this))
	{
		AnmExecBlend();
		UpdateFrame();
	}
	return baseExecute(pParam);
}

// DOL 0x8011510c [audited: matrix build and node pushes checked]
void cTaskCharGolvis::Draw(int nStatus, cJetTask* pParam)
{
	MonsterRender();
	cTaskModel::cPBModel* cPBModel = pCorrectModel();
	::Constants::jetGeoVecCopy(cPBModel->vTrans(), vTrans());
	::Constants::jetGeoVecCopy(cPBModel->vRotate(), vRotate());
	::Constants::jetGeoVecCopy(cPBModel->vScale(), vScale());
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, vRotate());
	Matrix4x4 _discard60;
	::Constants::jetGeoMtx44to34(_discard60, m);
	for (uint num = 0u; num < 5; num++)
	{
		DummyNW4R::SetMtx(pPBModel(num)->pScnMdl(), m);
		DummyNW4R::SetScale(pPBModel(num)->pScnMdl(), cPBModel->vScale()[0], cPBModel->vScale()[1], cPBModel->vScale()[2]);
		if (pPBModel(num) == cPBModel)
		{
			if (!pPBModel(num)->IsVisible())
			{
				pPBModel(num)->Visible();
			}
		}
		else if (pPBModel(num)->IsVisible())
		{
			pPBModel(num)->Invisible();
		}
	}
	cPBModel->UpdateMatrix();
	if (m_hPtkConf->IsValid())
	{
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecSet(v, 0.0f, 2.2f, 0.4f, 1.0f);
		m_hPtkConf->vSetTrans(v);
		// FUN_8008a324(+0x910, +0x820): +0x820 = the world matrix the node callback FUN_801153ac stores (head joint ATAMA_jnt).
		// Natively: the drawn model's world * the posed node (43rd session; was a Unity TODO = identity matrix)
		float w[16], n[12];
		if (cPBModel->NodeMatrix("ATAMA_jnt", n))
		{
			cPBModel->CalcWorld(w);   // column major
			for (int r = 0; r < 3; r++)
			{
				for (int c = 0; c < 4; c++)
				{
					m_PtkConfMtx[r][c] = w[r] * n[c] + w[4 + r] * n[4 + c] + w[8 + r] * n[8 + c] + (c == 3 ? w[12 + r] : 0.0f);
				}
			}
			m_PtkConfMtx[3][0] = m_PtkConfMtx[3][1] = m_PtkConfMtx[3][2] = 0.0f;
			m_PtkConfMtx[3][3] = 1.0f;
		}
		m_hPtkConf->UpdateMatrix(m_PtkConfMtx);
	}
	if (m_hPtkSweat->IsValid())
	{
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecSet(v2, 0.0f, 2.2f, 0.0f, 1.0f);
		::Constants::jetGeoVecAdd(v2, v2, vTransOld());
		m_hPtkSweat->vSetTrans(v2);
	}
	if (nType() != 2 && nType() != 3)
	{
		m_hPtkSweat->Detach(true);
	}
}

void cTaskCharGolvis::SetConfEffect()
{
	Vector4 v = Vector4::zero;
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, vTrans());
	::Constants::jetGeoVecSet(v, 0.0f, 5.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecAdd(v2, v2, v);
	Constants::PACN_APP()->SetEffect(22u, v2, Vector4(), m_hPtkConf);
}

void cTaskCharGolvis::EraseConfEffect()
{
	m_hPtkConf->Detach(true);
	pPBModel();
	if (nullptr != m_hSndPiyori)
	{
		SoundManager::get_SE()->Stop(m_hSndPiyori);
	}
}

// DOL 0x8011560c [audited]
void cTaskCharGolvis::SetModel()
{
	for (int i = 0; i < 5; i++)
	{
		cKingdomModel* obj = CreateModel((uint)i);
		obj->RegistModel(Constants::s_aModelName[i]);
		obj->IsDrawShadowDepth(true);
		obj->SetLightSetIdx(2);
		obj->SetFogIdx(2);
		obj->Visible();
	}
	pCurrentModel()->Invisible();
}

// DOL 0x801156cc [audited]
cTaskModel::cPBModel* cTaskCharGolvis::pCorrectModel()
{
	if (!CustomKingdomUtil::IF(m_iDispColor) || nType() == 4)
	{
		return pCurrentModel();
	}
	return pPBModel(nType() + 1);
}

// DOL 0x801156f4 [audited]
void cTaskCharGolvis::SetAnmState(const uint& state)
{
	SetAnmState(state, 0u);
}

// DOL 0x80115708 [audited]
void cTaskCharGolvis::SetAnmState(const uint& state, const uint& nTime)
{
	m_nAnmNew = state;
	m_hPtkStorm->Detach(true);
	m_hPtkStorm2->Detach(true);
	if (m_nAnmNew != m_nAnmNow)
	{
		m_nAnmOld = m_nAnmNow;
		m_nAnmNow = m_nAnmNew;
		if (nTime != 0)
		{
			m_nAnmChgCount = 0u;
			m_nAnmChgTime = nTime;
		}
		else
		{
			m_nAnmChgCount = 0u;
			m_nAnmChgTime = 1u;
		}
		m_nAnmSw ^= 1u;
		SetAnm(Constants::s_aAnimeName[m_nAnmNow], m_nAnmSw);
	}
}

// DOL 0x801157cc [audited]
uint cTaskCharGolvis::GetAnmState()
{
	return m_nAnmNow;
}

// DOL 0x801157d4 [audited]
void cTaskCharGolvis::SetAnm(string anime, uint idx)
{
	AnimationClip* animationClip = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(anime));
	CustomKingdomUtil::Assert(nullptr != animationClip, "nullpo");
	pModel(nType())->RegistAnm(idx, animationClip);
	if (nType() != 4)
	{
		pModel(nType() + 1)->RegistAnm(idx, animationClip);
	}
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE073_KAITEN);
}

// DOL 0x80115888 [audited]
float cTaskCharGolvis::GetAnmRate(uint idx)
{
	return pModel(nType())->GetWeightAnmChr(idx);
}

// DOL 0x801158c0 [audited]
void cTaskCharGolvis::AnmExecBlend()
{
	if (m_nAnmChgTime != m_nAnmChgCount)
	{
		m_nAnmChgCount++;
		float num = (float)m_nAnmChgCount / (float)m_nAnmChgTime;
		pModel(nType())->SetWeightAnmChr(m_nAnmSw, num);
		pModel(nType())->SetWeightAnmChr(m_nAnmSw ^ 1, 1.0f - num);
		if (nType() != 4)
		{
			pModel(nType() + 1)->SetWeightAnmChr(m_nAnmSw, num);
			pModel(nType() + 1)->SetWeightAnmChr(m_nAnmSw ^ 1, 1.0f - num);
		}
	}
}

// DOL 0x801159b8 [audited]
bool cTaskCharGolvis::IsAnmEnd()
{
	if (pModel(nType())->GetFrameMaxAnmChr(m_nAnmSw) <= pModel(nType())->GetFrameAnmChr(m_nAnmSw))
	{
		return true;
	}
	return false;
}

// DOL 0x80115a20 [audited]
cTaskCharGolvis* cTaskCharGolvis::Create(cJetTask* pParent, uint nRound)
{
	cTaskCharGolvis* result = nullptr;
	switch (nRound)
	{
	case 8u:
		result = new cTaskCharGolvis01(pParent);
		break;
	case 13u:
		result = new cTaskCharGolvis02(pParent);
		break;
	case 19u:
		result = new cTaskCharGolvis03(pParent);
		break;
	case 24u:
		result = new cTaskCharGolvis04(pParent);
		break;
	case 25u:
		result = new cTaskCharGolvis05(pParent);
		break;
	}
	return result;
}

// DOL 0x80115cf4 [audited: CSV names/defaults, constants 1.4/1.45/5/999/200/180 vs decomp]
void cTaskCharGolvis::Setting(bool bRelocate)
{
	if (bRelocate)
	{
		(void)(m_bExistPiyoriEffect);
		(void)(m_bExistCycloneSE);
		(void)(m_bExistGuitarSE);
		Constants::PACN_APP()->pTaskItem()->SetBossCookie(0u);
	}
	m_bLive = true;
	m_bDisp = true;
	m_bDeathAnime = false;
	m_bEnableGravity = false;
	m_bOnFloor = true;
	for (int i = 0; i < (int)std::size(m_iCount); i++)
	{
		m_iCount[i] = 0;
	}
	for (int j = 0; j < (int)std::size(m_fCount); j++)
	{
		m_fCount[j] = 0.0f;
	}
	m_iBlinkCount = 0;
	m_iDispColor = 0;
	m_iIzikeLevel = 0;
	m_iEatenNum = 0;
	m_iIzikeCount = 0;
	::Constants::jetGeoVecCopy(m_vFirstTrans, vTrans());
	::Constants::jetGeoVecCopy(m_vFirstRotate, vRotate());
	m_fRadius = 1.4f;
	m_fRadiusHit = 1.45f;
	m_fHeight = 5.0f;
	m_fLowHeight = 0.0f;
	m_iAIMode = 2;
	m_iSubState = 0;
	std::fill(std::begin(m_iAttackTime), std::end(m_iAttackTime), 0);
	m_bDispBall = false;
	m_iEffectTimer = 0;
	m_bExistPiyoriEffect = false;
	m_bExistCycloneSE = false;
	m_iCycloneWindSECount = 0;
	m_bExistGuitarSE = false;
	m_iMotionChangeTimer = 0;
	m_uiPacLife = Constants::PACN_APP()->pTaskCharPacman()->nHP();
	m_bPacDamage = false;
	m_bLaughEnable = false;
	m_pAttackLand = nullptr;
	cKingdomCSV* cKingdomCSV2 = new cKingdomCSV();
	cKingdomCSV2->RegistData("BIN_CSV_BOSS_PARAM");
	int num = cKingdomCSV2->SearchRow(0, "NORMAL_SPEED");
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fMoveSpeed), "error");
	num = cKingdomCSV2->SearchRow(0, "IJIKE_SPEED");
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fBijikeMoveSpeed), "error");
	num = cKingdomCSV2->SearchRow(0, "NORMAL_RANGE");
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fHitRange), "error");
	num = cKingdomCSV2->SearchRow(0, "IJIKE_RANGE");
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fBijikeHitRange), "error");
	if (0 <= (num = cKingdomCSV2->SearchRow(0, "FAN_POWER")))
	{
		CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fFanPower), "error");
	}
	else
	{
		m_fFanPower = 0.008f;
	}
	if (0 <= (num = cKingdomCSV2->SearchRow(0, "FAN_LENGTH")))
	{
		CustomKingdomUtil::Assert(cKingdomCSV2->F32(num, 1, m_fWindGuiterLength), "error");
	}
	else
	{
		m_fWindGuiterLength = 1.0f;
	}
	m_fBombRange = 999.0f;
	m_iBombInterval = 0;
	m_iBombAimTime = 180;
	m_fBombSightMoveSpeed = 200.0f;
	m_fMoveSpeedNow = 0.0f;
}

// DOL 0x80116028 [audited]
void cTaskCharGolvis::ChangeToFree()
{
	nType() = 0u;
	SetAnmState(0u, 10u);
	pHitChar(0)->FlagOn(2u);
	for (int i = 0; i < 8; i++)
	{
		Constants::gmSGolvisAttack* attackKind = GetAttackKind((uint)m_iEatenNum, (uint)m_iIzikeLevel, i);
		int max = attackKind->iTimeMax - attackKind->iTimeMin + 1;
		m_iAttackTime[i] = (int)((uint)MslRand() % (uint)max) + attackKind->iTimeMin + 1;   // DOL rand() % range (unsigned)
	}
	m_iAIMode = 2;
}

// DOL 0x80116118 [audited]
void cTaskCharGolvis::ChangeToEscape()
{
	nType() = 2u;
	SetAnmState(30u);
	m_iAIMode = 3;
	pHitChar(0)->FlagOff(2u);
	pHitChar(0)->SetRange(m_fBijikeHitRange);
	m_iEffectTimer = 0;
	::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
	SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE086_BOSS_IJIKE_3, false, pPBModel(nType())->pScnMdl());
}

// DOL 0x80116204 [audited]
void cTaskCharGolvis::ChangeToDamage()
{
	nType() = 0u;
	SetAnmState(7u);
	m_iAIMode = 4;
	pHitChar(0)->SetRange(m_fHitRange);
	::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE075_ENSOU);
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE073_KAITEN);
	for (int i = 0; i < 8; i++)
	{
		Constants::gmSGolvisAttack* attackKind = GetAttackKind((uint)m_iEatenNum, (uint)m_iIzikeLevel, i);
		int max = attackKind->iTimeMax - attackKind->iTimeMin + 1;
		m_iAttackTime[i] = (int)((uint)MslRand() % (uint)max) + attackKind->iTimeMin + 1;   // DOL rand() % range (unsigned)
	}
	Constants::PACN_APP()->pTaskConsole()->ButtCursorEnd();
	if (m_hPtkStorm->IsValid())
	{
		m_hPtkStorm->Detach(true);
	}
	if (m_hPtkStorm2->IsValid())
	{
		m_hPtkStorm2->Detach(true);
	}
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE026_KAZE);
	SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE098_TATAKI, false, pPBModel(nType())->pScnMdl());
}

void cTaskCharGolvis::ChangeToPossess()
{
	CustomKingdomUtil::Assert("botsu");
}

void cTaskCharGolvis::ChangeToKnock()
{
	CustomKingdomUtil::Assert("botsu");
}

// DOL 0x801163e0 [audited: every kind -> state/time, carry-ghost test, IsGround, aim cursor vs decomp; EraseConf model flag gap as in Execute]
bool cTaskCharGolvis::ChangeToAttack(int iAttackKind)
{
	uint state = 0u;
	uint nTime = 0u;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	switch (iAttackKind)
	{
	case 1:
	case 2:
	case 3:
		state = 14u;
		break;
	case 4:
	case 5:
	case 6:
		state = 13u;
		break;
	case 8:
	case 9:
		state = 21u;
		nTime = 10u;
		break;
	case 10:
	case 11:
	case 12:
		if (!Constants::PACMAN_OBJ()->IsGround())
		{
			return false;
		}
		::Constants::jetGeoVecCopy(m_vPos[1], v);
		::Constants::jetGeoVecCopy(m_vPos[0], vTrans());
		::Constants::jetGeoVecCopy(m_vPos[2], m_vPos[1]);
		state = 17u;
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE064_AIM_HYOUJI, false, pPBModel(nType())->pScnMdl());
		Constants::PACN_APP()->pTaskConsole()->ButtCursorStart(v);
		break;
	case 14:
		state = 24u;
		nTime = 10u;
		if (!m_hPtkConf->IsValid())
		{
			SetConfEffect();
		}
		if (!SoundManager::get_SE()->IsPlaying(m_hSndPiyori))   // DOL: the loop handle +0x9b0 is not playing (was a null test: stopped handles stay non-null)
		{
			if (cPacnRollLoopSndMan::CanPlay()) m_hSndPiyori = cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE051_PIYORI, true, pPBModel(nType())->pScnMdl()));
		}
		break;
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
		{
			sbyte* array = Constants::iCarryGhostSet[iAttackKind - 20];   // DOL 0x8043ee64 [audited: table bytes verified] + (kind - 0x14) * 3
			int num = ((array[1] != -1) ? 1 : 0);
			int num2 = ((array[2] != -1) ? 1 : 0);
			int num3 = 1 + num + num2;
			if (4 < (long)Constants::PACN_APP()->EnemyGetActiveNum() + (long)num3)
			{
				return false;
			}
			for (int i = 0; i < 3 && array[i] != -1; i++)
			{
				if (Constants::PACN_APP()->EnemyIsActive((uint)array[i]))
				{
					return false;
				}
			}
			state = 12u;
			break;
		}
	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
	case 109:
	case 110:
	case 111:
	case 112:
	case 113:
	case 114:
	case 115:
	case 116:
	case 117:
	case 118:
	case 119:
	case 120:
	case 121:
	case 122:
	case 123:
	case 124:
	case 125:
		AttackLand(iAttackKind);
		return true;
	}
	nType() = 0u;
	SetAnmState(state, nTime);
	m_iAIMode = 7;
	m_iSubState = iAttackKind;
	m_iCount[0] = 0;
	m_iCount[1] = 0;
	m_iEffectTimer = 0;
	if (iAttackKind != 14)
	{
		EraseConfEffect();
	}
	return true;
}

// DOL 0x80116778 [audited]
void cTaskCharGolvis::ChangeToMotion(uint uiMotGID)
{
	nType() = 0u;
	SetAnmState(uiMotGID);
	m_iAIMode = 8;
}

// DOL 0x801167c0 [audited]
void cTaskCharGolvis::ChangeToAppear()
{
	nType() = 0u;
	SetAnmState(3u);
	m_iAIMode = 11;
	pHitChar(0)->SetRange(m_fHitRange);
	pHitChar(0)->FlagOff(2u);
	// DOL: m03se_099_Gol_Toujyou starts here, on the boss's sound actor (+0x860) (43rd session: the C# deferred it to a Unity
	// coroutine that the port never ran, so the sound was missing)
	SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE099_GOL_TOUJYOU, false, pPBModel(nType())->pScnMdl());
	m_iCount[0] = 0;
	m_iCount[1] = 0;
}

IEnumerator* cTaskCharGolvis::WaitOneFramePlaySE()
{
	// TODO(unity): coroutine; original C# body:
	// 	yield return null;
	// 	yield return new WaitWhile(() => Constants.IS_PAC_PAUSE());
	// 	SoundManager.SE.PlaySE(ESEKind.PACN_ROLL_SND_SE099_GOL_TOUJYOU);
	return nullptr;
}

// DOL 0x80116898 [audited]
void cTaskCharGolvis::ChangeToRevive()
{
	::Constants::jetGeoVecCopy(vTrans(), m_vFirstTrans);
	::Constants::jetGeoVecCopy(vRotate(), m_vFirstRotate);
	m_iIzikeLevel = 0;
	m_iIzikeCount = 0;
	nType() = 0u;
	SetAnmState(10u);
	pHitChar(0)->FlagOff(2u);
	pHitChar(0)->SetRange(m_fHitRange);
	m_iAIMode = 12;
	::Constants::jetGeoVecSet(vScale(), 0.0f, 0.0f, 0.0f, 1.0f);
	Constants::PACN_APP()->SetEffect(23u, vTrans(), Vector4());
	m_hPtkSweat->Detach(true);
	m_iCount[0] = 0;
	m_iCount[1] = 0;
	SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE084_BOSS_IJIKE_1, false, pPBModel(nType())->pScnMdl());
}

// DOL 0x80116a00 [audited]
void cTaskCharGolvis::ChangeToMedama()
{
	::Constants::jetGeoVecSet(vScale(), 0.0f, 0.0f, 0.0f, 1.0f);
	m_pBijikeEyeObj->StartEye(vTrans());
	m_iCount[0] = 0;
	m_iCount[1] = 0;
	m_iAIMode = 10;
	Constants::PACN_APP()->pTaskItem()->SetBossCookie(0u);
	if (m_hPtkSweat->IsValid())
	{
		m_hPtkSweat->Detach(true);
	}
}

// DOL 0x80116a90 [audited: checked in the 43rd session incl. the out-of-table float read]
bool cTaskCharGolvis::TryChangeToAttack()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (m_bEnableGravity && !m_bOnFloor)
	{
		return false;
	}
	float num = ::Constants::bizGeoVecDistance2XZ(vTrans(), v);
	float num2 = ::Constants::JET_ABS(vTrans()[1] - v[1]);
	for (int i = 0; i < 8; i++)
	{
		Constants::gmSGolvisAttack* attackKind = GetAttackKind((uint)m_iEatenNum, (uint)m_iIzikeLevel, i);
		if (attackKind == nullptr || attackKind->uiKind == 0)
		{
			break;
		}
		// DOL: kinds < 0x14 index the 15-float table 0x8043ee28; kinds 15..19 (BOOT_0..4) read past it into the next table
		// (iCarryGhostSet's bytes as big-endian floats: tiny / NaN / negative values). 43rd session: emulated, was out of bounds / skipped.
		float permit = 0.0f;
		if (attackKind->uiKind < 20)
		{
			if (attackKind->uiKind < (int)std::size(Constants::iAttackPermitSubY))
			{
				permit = Constants::iAttackPermitSubY[attackKind->uiKind];
			}
			else
			{
				const sbyte* b = &Constants::iCarryGhostSet[0][0] + (attackKind->uiKind - (int)std::size(Constants::iAttackPermitSubY)) * 4;
				uint32_t w = ((uint32_t)(byte)b[0] << 24) | ((uint32_t)(byte)b[1] << 16) | ((uint32_t)(byte)b[2] << 8) | (uint32_t)(byte)b[3];
				std::memcpy(&permit, &w, 4);
			}
		}
		bool flag = !(num < attackKind->fRangeMin * attackKind->fRangeMin || attackKind->fRangeMax * attackKind->fRangeMax <= num
			|| (attackKind->uiKind < 20 && permit != 0.0f && permit < num2));
		if (m_iAttackTime[i] == 0 && flag)
		{
			int max = attackKind->iTimeMax - attackKind->iTimeMin + 1;
			m_iAttackTime[i] = (int)((uint)MslRand() % (uint)max) + attackKind->iTimeMin + 1;   // DOL rand() % range (unsigned)
		}
		if (0 < m_iAttackTime[i] || IsForceAttack())
		{
			m_iAttackTime[i]--;
			if (m_iAttackTime[i] < 0)
			{
				m_iAttackTime[i] = 0;
			}
			if ((m_iAttackTime[i] == 0 || IsForceAttack()) && (flag || IsForceAttack()) && ChangeToAttack(attackKind->uiKind))
			{
				pHitChar(0)->FlagOn(2u);
				return true;
			}
		}
	}
	return false;
}

// DOL 0x80116d3c [audited]
bool cTaskCharGolvis::TryChangeToLaugh()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (!m_bPacDamage && !m_bLaughEnable)
	{
		return false;
	}
	m_bLaughEnable = false;
	if (400.0f < ::Constants::bizGeoVecDistance2XZ(vTrans(), cTaskCharPacman2->vTrans()))
	{
		return false;
	}
	if ((MslRand() & 1) != 0)   // DOL: rand() & 1
	{
		return false;
	}
	ChangeToMotion(6u);
	return true;
}

// DOL 0x80116de8 [audited: 43rd-session pass only (per-sub-state comments); not re-read line by line]
void cTaskCharGolvis::AttackProcess()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	uint anmState = GetAnmState();
	float frameAnmChr = pModel(nType())->GetFrameAnmChr(m_nAnmSw);
	float frameMaxAnmChr = pModel(nType())->GetFrameMaxAnmChr(m_nAnmSw);
	// frameAnmChr *= 60.0f;  C# Unity seconds -> frames; native frames already (the DOL compares raw frames)
	// frameMaxAnmChr *= 60.0f;  (same)
	Vector4 v;
	::Constants::jetGeoVecCopy(v, cTaskCharPacman2->vTrans());
	float pOut;
	switch (m_iSubState)
	{
	case 1:
	case 2:
	case 3:
		if (anmState == 14)
		{
			pHitChar(0)->FlagOff(2u);
			if (frameAnmChr <= 10.0f)
			{
				::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
				vRotate()[1] = pOut;
			}
			if (IsAnmEnd())
			{
				int num = 0;
				switch (m_iSubState)
				{
				case 1:
					num = 60;
					break;
				case 2:
					num = 30;
					break;
				case 3:
					num = 0;
					break;
				}
				if (num <= ++m_iCount[0])
				{
					m_iCount[0] = 0;
					SetAnmState(16u);
				}
			}
			break;
		}
		pHitChar(0)->FlagOn(2u);
		// DOL: the hit runs while 5 <= frame <= 5.1 (the C# used a one-shot flag)
		if (5.0f <= frameAnmChr && frameAnmChr <= 5.1f)
		{
			Vector4 pvLocalAttackPos = Vector4(0.5f, 0.0f, 3.5f, 1.0f);   // DAT_80378088
			if (Attack(pvLocalAttackPos, 2.0f, vTrans(), 0.2f, true) == 1)
			{
				m_bLaughEnable = true;
			}
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE080_BOSS_TATAKI, false, pPBModel(nType())->pScnMdl());
		}
		if (IsAnmEnd())
		{
			if (!TryChangeToLaugh())
			{
				ChangeToFree();
			}
		}
		break;
	case 4:
	case 5:
	case 6:
		{
			if (anmState == 13)
			{
				pHitChar(0)->FlagOff(2u);
				if (frameAnmChr <= 10.0f)
				{
					::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
					vRotate()[1] = pOut;
				}
				if (IsAnmEnd())
				{
					int num6 = 0;
					switch (m_iSubState)
					{
					case 4:
						num6 = 60;
						break;
					case 5:
						num6 = 30;
						break;
					case 6:
						num6 = 0;
						break;
					}
					if (num6 <= ++m_iCount[0])
					{
						m_iCount[0] = 0;
						SetAnmState(15u);
					}
				}
				MonsterFrameMove_Free();
				break;
			}
			pHitChar(0)->FlagOn(2u);
			// DOL: the SE on frame == 3, the hit (and a 5x hit range) while 8 <= frame <= 9 (the C# used one-shot flags and also
			// switched hit 1 off, which the DOL does not)
			if (frameAnmChr == 3.0f)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE074_TATAKI, false, pPBModel(nType())->pScnMdl());
			}
			if (frameAnmChr < 8.0f || 9.0f < frameAnmChr)
			{
				pHitChar(0)->SetRange(m_fHitRange);
			}
			else
			{
				Vector4 pvLocalAttackPos2 = Vector4(0.0f, 1.5f, 1.0f, 0.0f);   // DAT_80378098
				if (Attack(pvLocalAttackPos2, 4.0f, vTrans(), 0.4f, true) == 1)
				{
					m_bLaughEnable = true;
				}
				pHitChar(0)->SetRange(5.0f * m_fHitRange);
			}
			if (IsAnmEnd())
			{
				if (!TryChangeToLaugh())
				{
					ChangeToFree();
				}
			}
			break;
		}
	case 8:
	case 9:
		switch (m_iCount[0])
		{
		case 0:
			::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
			vRotate()[1] = pOut;
			if (IsAnmEnd())
			{
				SetAnmState(22u);
				if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE073_KAITEN, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY, pPBModel(nType())->pScnMdl()));
				m_iCount[0]++;
			}
			break;
		case 1:
			{
				::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(1.5f));
				vRotate()[1] = pOut;
				float fAng = Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]);
				Vector4 v2 = Vector4();
				float x = vTrans()[1] - (v[1] - 0.4f);
				x = ::Constants::JET_ABS(x);
				if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE073_KAITEN, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY, pPBModel(nType())->pScnMdl()));
				Vector4 v4;
				Vector4 v5;
				if (m_iSubState == 8)
				{
					Vector4 v3 = Vector4(0.0f, 2.5f, 20.0f, 1.0f);
					v2[0] = 0.0f - Mathf::Sin(vRotate()[1]);
					v2[2] = 0.0f - Mathf::Cos(vRotate()[1]);
					v2[1] = 0.0f;
					v2[3] = 1.0f;
					::Constants::jetGeoVecCopy(v4, vRotate());
					::Constants::jetGeoVecRotY(v5, v3, vRotate()[1]);
					::Constants::jetGeoVecAdd(v5, v5, vTrans());
					v4[1] += ::Constants::JET_DEG2RAD(180.0f);
					if (!m_hPtkStorm2->IsValid())
					{
						Constants::PACN_APP()->SetEffect(16u, v5, v4, m_hPtkStorm2);
					}
					else
					{
						m_hPtkStorm2->vSetTrans(v5);
						m_hPtkStorm2->vSetRotate(v4);
					}
					if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE026_KAZE, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY, pPBModel(nType())->pScnMdl()));
				}
				else
				{
					Vector4 v6 = Vector4(1.2f, 2.5f, 5.0f, 1.0f);
					v2[0] = Mathf::Sin(vRotate()[1]);
					v2[2] = Mathf::Cos(vRotate()[1]);
					v2[1] = 0.0f;
					v2[3] = 1.0f;
					::Constants::jetGeoVecCopy(v4, vRotate());
					::Constants::jetGeoVecRotY(v5, v6, vRotate()[1]);
					::Constants::jetGeoVecAdd(v5, v5, vTrans());
					if (!m_hPtkStorm->IsValid())
					{
						Constants::PACN_APP()->SetEffect(16u, v5, v4, m_hPtkStorm);
					}
					else
					{
						m_hPtkStorm->vSetTrans(v5);
						m_hPtkStorm->vSetRotate(v4);
					}
					if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE026_KAZE, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY, pPBModel(nType())->pScnMdl()));
				}
				if (::Constants::bizMathGetRad(vRotate()[1], fAng) <= ::Constants::JET_DEG2RAD(15.0f) && ::Constants::bizGeoVecDistance2XZ(vTrans(), v) <= 225.0f * m_fWindGuiterLength && x <= Constants::iAttackPermitSubY[m_iSubState])
				{
					float num5 = ::Constants::jetGeoVecLength(v2);
					if (CustomKingdomUtil::IF(num5))
					{
						::Constants::jetGeoVecScale(v2, v2, m_fFanPower / num5);
						::Constants::jetGeoVecAdd(Constants::PACMAN_OBJ()->vMove(), Constants::PACMAN_OBJ()->vMove(), v2);
					}
				}
				m_iCount[1]++;
				if (240 <= m_iCount[1])
				{
					ChangeToAttack(14);
				}
				break;
			}
		case 2:
			if (frameAnmChr >= frameMaxAnmChr)
			{
				ChangeToFree();
			}
			break;
		}
		break;
	case 10:
	case 11:
	case 12:
		switch (m_iCount[0])
		{
		case 0:
			{
				pHitChar(0)->FlagOff(2u);
				float num3 = 30.0f;
				float num4 = 44.0f;
				if (frameAnmChr < num3)
				{
					if (cTaskCharPacman2->IsGround())
					{
						::Constants::jetGeoVecCopy(m_vPos[1], CalcButtCursorTrans(v));
						::Constants::jetGeoVecCopy(m_vPos[2], m_vPos[1]);
						Constants::PACN_APP()->pTaskConsole()->ButtCursorUpdate(CalcButtCursorTrans(v));
					}
					::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(m_vPos[1][0] - vTrans()[0], m_vPos[1][2] - vTrans()[2]), ::Constants::JET_DEG2RAD(15.0f));
					vRotate()[1] = pOut;
				}
				else
				{
					if (cTaskCharPacman2->IsGround() && ::Constants::bizGeoVecDistance2XZ(vTrans(), cTaskCharPacman2->vTrans()) <= 100.0f)
					{
						::Constants::jetGeoVecCopy(m_vPos[2], CalcButtCursorTrans(v));
						Constants::PACN_APP()->pTaskConsole()->ButtCursorUpdate(CalcButtCursorTrans(v));
					}
					if (frameAnmChr == num3)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE076_BOSS_JUMP, false, pPBModel(nType())->pScnMdl());
					}
					if (frameAnmChr <= num4)
					{
						m_iCount[1]++;
					}
				}
				float fRate = (float)m_iCount[1] * 0.5f / (num4 - num3);
				if (1.0f < fRate)
				{
					fRate = 1.0f;
				}
				vTrans()[0] = ::Constants::bizMathLerpFloat(m_vPos[1][0], m_vPos[0][0], fRate);
				vTrans()[2] = ::Constants::bizMathLerpFloat(m_vPos[1][2], m_vPos[0][2], fRate);
				vTrans()[1] = ::Constants::bizMathLerpFloat(m_vPos[1][1], m_vPos[0][1], fRate);
				vTrans()[1] += Mathf::Sin(::Constants::JET_DEG2RAD(90.0f * fRate)) * 10.0f;
				if (frameAnmChr >= frameMaxAnmChr)
				{
					::Constants::jetGeoVecCopy(m_vPos[0], vTrans());
					m_iCount[0]++;
					m_iCount[1] = 0;
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE065_AIM_KOTEI, false, pPBModel(nType())->pScnMdl());
					Constants::PACN_APP()->pTaskConsole()->ButtCursorRed();
				}
				break;
			}
		case 1:
			::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(m_vPos[1][0] - vTrans()[0], m_vPos[1][2] - vTrans()[2]), ::Constants::JET_DEG2RAD(18.0f));
			vRotate()[1] = pOut;
			m_iCount[1]++;
			if ((m_iSubState == 10 && m_iCount[1] == 40) || (m_iSubState == 11 && m_iCount[1] == 20) || (m_iSubState == 12 && m_iCount[1] == 1))
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE077_BOSS_RAKKA_1, false, pPBModel(nType())->pScnMdl());
				::Constants::jetGeoVecCopy(m_vPos[1], m_vPos[2]);
			}
			break;
		case 2:
			{
				::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(m_vPos[1][0] - vTrans()[0], m_vPos[1][2] - vTrans()[2]), ::Constants::JET_DEG2RAD(18.0f));
				vRotate()[1] = pOut;
				m_iCount[1]++;
				float num2 = (float)m_iCount[1] * 1.0f / 15.0f;
				vTrans()[0] = m_vPos[0][0] + (m_vPos[1][0] - m_vPos[0][0]) * num2;
				vTrans()[2] = m_vPos[0][2] + (m_vPos[1][2] - m_vPos[0][2]) * num2;
				vTrans()[1] = m_vPos[0][1] + (Mathf::Cos(::Constants::JET_DEG2RAD(180.0f + 90.0f * num2)) + 1.0f) * (m_vPos[1][1] - m_vPos[0][1]);
				pHitChar(0)->FlagOff(2u);
				if (num2 == 1.0f)
				{
					m_iCount[0]++;
					m_iCount[1] = 0;
					SetAnmState(18u);
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE078_BOSS_RAKKA_2, false, pPBModel(nType())->pScnMdl());
					Constants::PACN_APP()->pTaskConsole()->ButtCursorEnd();
				}
				break;
			}
		case 3:
			if (++m_iCount[1] == 60)   // DOL 0x3c (the C# had 30)
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				SetAnmState(19u, 10u);
				m_iEffectTimer = 0;
			}
			if (m_iCount[1] <= 4)
			{
				pHitChar(0)->FlagOn(2u);
				pHitChar(0)->SetRange(m_fHitRange * 4.0f);
			}
			else
			{
				pHitChar(0)->SetRange(m_fHitRange / 2.0f);
			}
			break;
		case 4:
			pHitChar(0)->SetRange(m_fHitRange / 2.0f);
			if (++m_iCount[1] == 120)
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				SetAnmState(20u, 10u);
			}
			break;
		case 5:
			pHitChar(0)->SetRange(m_fHitRange / 2.0f);
			if (frameAnmChr == 8.0f)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE079_BOSS_RAKKA_3);
			}
			if (IsAnmEnd())
			{
				pHitChar(0)->SetRange(m_fHitRange);
				ChangeToFree();
			}
			break;
		}
		break;
	case 14:
		switch (m_iCount[0])
		{
		case 0:
			if (frameAnmChr + 1.0f >= frameMaxAnmChr)
			{
				m_iCount[0]++;
				SetAnmState(24u);
			}
			break;
		case 1:
			if (frameAnmChr + 1.0f >= frameMaxAnmChr && ++m_iCount[1] == 2)
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				SetAnmState(25u);
				EraseConfEffect();
			}
			break;
		case 2:
			if (IsAnmEnd())
			{
				SetAnmState(26u);
				m_iCount[0]++;
			}
			break;
		case 3:
			if (++m_iCount[1] == 100)
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				SetAnmState(27u);
			}
			break;
		case 4:
			if (IsAnmEnd())
			{
				ChangeToFree();
			}
			break;
		}
		break;
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
		if (frameAnmChr < 31.0f)
		{
			::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
			vRotate()[1] = pOut;
		}
		if (frameAnmChr == 25.0f)   // DOL FLOAT_8047bc78 (the C# used >= 25 + a one-shot flag)
		{
			sbyte* array = Constants::iCarryGhostSet[m_iSubState - 20];
			for (int i = 0; i < 3 && array[i] != -1; i++)
			{
				Constants::PACN_APP()->EnemyActive((uint)array[i]);
			}
		}
		if (IsAnmEnd())
		{
			ChangeToFree();
		}
		break;
	}
	if (m_iSubState != 9)
	{
		m_hPtkStorm->Detach(true);
	}
	if (m_iSubState != 8)
	{
		m_hPtkStorm2->Detach(true);
	}
	if (m_iSubState != 9 && m_iSubState != 8)
	{
		SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE026_KAZE);
	}
}

// DOL 0x80118500 [audited: 43rd-session pass only (case comments); not re-read line by line in the function-level pass]
void cTaskCharGolvis::MonsterFrameMove()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, cTaskCharPacman2->vTrans());
	float frameAnmChr = pModel(nType())->GetFrameAnmChr(m_nAnmSw);
	float frameMaxAnmChr = pModel(nType())->GetFrameMaxAnmChr(m_nAnmSw);
	// frameAnmChr *= 60.0f;  C# Unity seconds -> frames; native frames already (the DOL compares raw frames)
	// frameMaxAnmChr *= 60.0f;  (same)
	if (Constants::IS_NAKAJIMA_PAUSE(this))
	{
		return;
	}
	::Constants::jetGeoVecCopy(vTransOld(), vTrans());
	m_bEnableGravity = false;
	m_bDispBall = false;
	if (Constants::PACN_APP()->pTaskCharPacman()->nHP() < m_uiPacLife)
	{
		m_bPacDamage = true;
	}
	else
	{
		m_bPacDamage = false;
	}
	m_uiPacLife = Constants::PACN_APP()->pTaskCharPacman()->nHP();
	if (IsBijikeMsgCome())
	{
		m_iIzikeCount = 600;
		m_bLaughEnable = false;
		if (m_iIzikeLevel < 3)
		{
			if ((m_iAIMode != 11 && m_iAIMode != 12) || m_iIzikeLevel == 2)
			{
				ChangeToDamage();
			}
			m_iIzikeLevel++;
			if (m_iIzikeLevel == 3)
			{
				Constants::PACN_APP()->SetEffect(23u, vTrans(), Vector4());
			}
		}
		m_bIjikeMsgCome = false;
	}
	if (m_iIzikeLevel != 0 && !IsBossDeath() && --m_iIzikeCount == 0)
	{
		if (m_iAIMode == 4 || m_iAIMode == 5 || m_iAIMode == 6 || m_iAIMode == 7 || m_iAIMode == 8 || m_iAIMode == 9 || m_iAIMode == 10 || m_iAIMode == 14)
		{
			m_iIzikeCount = 1;
		}
		else
		{
			m_iIzikeLevel = 0;
			ChangeToMotion(5u);
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE101_GOL_IJIKE, false, pPBModel(nType())->pScnMdl());
			if (m_iIzikeLevel <= 2)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE084_BOSS_IJIKE_1, false, pPBModel(nType())->pScnMdl());
			}
			else
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE085_BOSS_IJIKE_2, false, pPBModel(nType())->pScnMdl());
			}
		}
	}
	if (HitCheckPacman() && m_iEatenNum < 3 && m_iAIMode != 5 && m_iAIMode != 6 && m_iAIMode != 9 && m_iAIMode != 10 && m_iAIMode != 4 && m_iAIMode != 14 && nType() == 2)
	{
		m_iAIMode = 9;
		m_iCount[0] = 0;
		m_fCount[0] = 0.0f;
		Constants::PACN_APP()->NakajimaPause(this);
		m_bDeathAnime = true;
	}
	MonsterFrameMove_PreProcess();
	switch (m_iAIMode)
	{
	case 2:
		MonsterFrameMove_Free();
		break;
	case 3:
		MonsterFrameMove_Escape();
		break;
	case 4:
		frameAnmChr = pModel(nType())->GetFrameAnmChr(m_nAnmSw);
		frameMaxAnmChr = pModel(nType())->GetFrameMaxAnmChr(m_nAnmSw);
		// frameAnmChr *= 60.0f;  C# Unity seconds -> frames; native frames already (the DOL compares raw frames)
		// frameMaxAnmChr *= 60.0f;  (same)
		if (m_iIzikeLevel == 3)
		{
			vScale()[0] = 1.0f - 0.55f * frameAnmChr / frameMaxAnmChr;
			vScale()[1] = vScale()[0];
			vScale()[2] = vScale()[0];
		}
		if (IsAnmEnd())
		{
			if (m_iIzikeLevel < 3)
			{
				ChangeToFree();
			}
			else
			{
				ChangeToEscape();
			}
			if (!m_hPtkSweat->IsValid())
			{
				Constants::PACN_APP()->SetEffect(13u, vTrans(), Vector4(), m_hPtkSweat);
			}
		}
		break;
	case 7:
		AttackProcess();
		break;
	case 8:
		{
			uint anmState = GetAnmState();
			frameAnmChr = pModel(nType())->GetFrameAnmChr(m_nAnmSw);
			// frameAnmChr *= 60.0f;  C# Unity seconds -> frames; native frames already (the DOL compares raw frames)
			if (anmState == 6)
			{
				float pOut;
				::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
				vRotate()[1] = pOut;
				if (frameAnmChr == 1.0f)
				{
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE103_GOL_2, false, pPBModel(nType())->pScnMdl());
				}
			}
			if (IsAnmEnd() && !IsBossDeath())
			{
				ChangeToFree();
			}
			break;
		}
	case 9:
		{
			if (!bUseDeathDemo() || m_iEatenNum != 2)
			{
				m_iCount[0]++;
				m_fCount[0] += 0.02f;
				vScale()[0] = (float)(25 - m_iCount[0]) / 25.0f;
				vScale()[1] = vScale()[0];
				vScale()[2] = vScale()[0];
				Vector4 v2;
				::Constants::jetGeoVecCopy(v2, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
				if (::Constants::bizMathNearTrans(vTrans(), vTrans(), v2, m_fCount[0]))
				{
					::Constants::jetGeoVecSet(vScale(), 0.0f, 0.0f, 0.0f, 1.0f);
				}
			}
			else
			{
				m_iCount[0] = 25;
			}
			if (m_iCount[0] != 25)
			{
				break;
			}
			m_iEatenNum++;
			bool flag = true;
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE017_GHOST, false, pPBModel(nType())->pScnMdl());
			if (IsBossDeath())
			{
				if (bUseDeathDemo())
				{
					m_iAIMode = 13;
					flag = false;
				}
				else
				{
					Constants::PACN_APP()->IsClear() = true;
				}
			}
			else
			{
				ChangeToMedama();
				m_bDeathAnime = false;
			}
			if (flag)
			{
				Constants::PACN_APP()->NakajimaPlay(this);
			}
			break;
		}
	case 10:
		if (m_pBijikeEyeObj->IsEndEye())
		{
			ChangeToRevive();
		}
		break;
	case 11:
		vRotate()[1] = Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]);
		if (m_iCount[0] == 0)
		{
			Constants::PACN_APP()->pTaskItem()->SetBossCookie(nLife());
			m_iCount[0]++;
		}
		if (pCurrentModel()->GetFrameAnmChr(m_nAnmSw) >= 50.0f)
		{
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE075_ENSOU, 3.3333333f);
		}
		if (frameAnmChr == 0.0f)
		{
			if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE075_ENSOU, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY, pPBModel(nType())->pScnMdl()));
		}
		if (frameAnmChr == 36.0f && m_bLaughEnable)
		{
			switch (MslRand() % 4)   // DOL rand() % 4 (signed)
			{
			case 0:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE100_GOL_HANYOU, false, pPBModel(nType())->pScnMdl());
				break;
			case 1:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE102_GOL, false, pPBModel(nType())->pScnMdl());
				break;
			case 2:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE103_GOL_2, false, pPBModel(nType())->pScnMdl());
				break;
			case 3:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE101_GOL_IJIKE, false, pPBModel(nType())->pScnMdl());
				break;
			}
		}
		if (IsAnmEnd())
		{
			InitStateStart();
			pHitChar(0)->FlagOn(2u);
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE075_ENSOU);
		}
		break;
	case 13:
		m_iIzikeLevel = 0;
		MonsterFrameMove_Death();
		break;
	case 12:
		if (m_iCount[0] == 0)
		{
			if (++m_iCount[1] == 30)
			{
				m_iCount[0]++;
				m_iCount[1] = 0;
				::Constants::jetGeoVecSet(vScale(), 0.45f, 0.45f, 0.45f, 1.0f);
				SetAnmState(4u);
			}
			break;
		}
		::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
		if (frameAnmChr == 38.0f)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE100_GOL_HANYOU, false, pPBModel(nType())->pScnMdl());
		}
		if (IsAnmEnd())
		{
			::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
			InitStateStart();
			pHitChar(0)->FlagOn(2u);
			Constants::PACN_APP()->pTaskItem()->SetBossCookie(nLife());
		}
		break;
	}
	if (m_iAIMode == 1 || m_iAIMode == 2 || m_iAIMode == 3)
	{
		if (m_iMotionChangeTimer < 20)
		{
			m_iMotionChangeTimer++;
		}
		else
		{
			uint anmState2 = GetAnmState();
			if (nType() == 0)
			{
				if (vTrans()[0] != vTransOld(0) || vTrans()[2] != vTransOld(2))
				{
					if (m_iEatenNum < 2)
					{
						if (anmState2 != 1)
						{
							SetAnmState(1u, 10u);
							m_iMotionChangeTimer = 0;
						}
					}
					else if (anmState2 != 2)
					{
						SetAnmState(2u, 10u);
						m_iMotionChangeTimer = 0;
					}
				}
				else if (anmState2 == 1)
				{
					SetAnmState(0u, 10u);
					m_iMotionChangeTimer = 0;
				}
			}
			else if (anmState2 != 30)
			{
				SetAnmState(30u, 10u);
				m_iMotionChangeTimer = 0;
			}
		}
	}
	else
	{
		m_iMotionChangeTimer = 20;
	}
	switch (nType())
	{
	case 0u:
		m_fRadius = 1.4f;
		m_fRadiusHit = 1.45f;
		m_fHeight = 5.0f;
		m_fLowHeight = 0.0f;
		break;
	case 2u:
		m_fRadius = 0.55f;
		m_fRadiusHit = 1.3f;
		m_fHeight = 2.0f;
		m_fLowHeight = 0.0f;
		break;
	}
	if (m_iAIMode != 7 && (vTrans()[0] != vTransOld(0) || vTrans()[2] != vTransOld(2)))
	{
		float pOut2;
		::Constants::bizMathNearRad(pOut2, vRotate()[1], Mathf::Atan2(vTrans()[0] - vTransOld(0), vTrans()[2] - vTransOld(2)), ::Constants::JET_DEG2RAD(6.0f));
		vRotate()[1] = pOut2;
	}
}

// DOL 0x801192c8 [audited]
void cTaskCharGolvis::MonsterRender()
{
	if (Constants::IS_PAC_PAUSE() || Constants::PACMAN_OBJ()->IsMiss())
	{
		return;
	}
	switch (m_iAIMode)
	{
	case 3:
		m_iDispColor = 1;
		if (180 < m_iIzikeCount)
		{
			m_iBlinkCount = 0;
			return;
		}
		m_iBlinkCount = (m_iBlinkCount + 1) % 30;
		if (m_iBlinkCount / 15 == 0)
		{
			m_iDispColor = 0;
		}
		return;
	case 9:
		m_iDispColor = 1;
		return;
	case 10:
		m_iDispColor = 1;
		return;
	case 14:
		m_iDispColor = 0;
		return;
	}
	m_iDispColor = 0;
	if (m_iIzikeLevel == 0)
	{
		m_iBlinkCount = 0;
		return;
	}
	m_iBlinkCount = (m_iBlinkCount + 1) % 120;
	if (m_iIzikeLevel == 1)
	{
		if (m_iBlinkCount / 2 % 15 <= 2)
		{
			m_iDispColor = 1;
		}
	}
	else if (2 <= m_iIzikeLevel && m_iBlinkCount % 12 < 6)
	{
		m_iDispColor = 1;
	}
}

// DOL 0x801194b8 [audited]
int cTaskCharGolvis::Attack(Vector4 pvLocalAttackPos, float fAttackR, Vector4 pvFlipCenterPos, float fAttackSpeed, bool bKabutoGuardEnable)
{
	pHitChar(1)->SetOfsPos(pvLocalAttackPos);
	pHitChar(1)->SetRange(fAttackR);
	pHitChar(1)->SetBoundCenter(pvFlipCenterPos);
	pHitChar(1)->FlagOn((uint)(0x21 | (bKabutoGuardEnable ? 16 : 0)));   // (the C# also set flag 2: already on since the ctor, the DOL does not)
	HitUpdate();
	if (Constants::PACN_APP()->pTaskCollision()->HitPreCheck(pHitChar(1)))
	{
		if (!bKabutoGuardEnable || Constants::PACN_APP()->pTaskCharPacman()->nType() != 2)
		{
			return 1;
		}
		return 2;
	}
	return 0;
}

void cTaskCharGolvis::RequestEffectAround(int iKind, int iInterval)
{
	if (iInterval != 0)
	{
		(void)(m_iEffectTimer);
	}
	if (0 < iInterval)
	{
		m_iEffectTimer++;
		if (iInterval <= m_iEffectTimer)
		{
			m_iEffectTimer = 0;
		}
	}
}

// DOL 0x80119608 [audited]
bool cTaskCharGolvis::HitCheckPacman()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if ((double)::Constants::jetGeoVecDistance(vTrans(), cTaskCharPacman2->vTrans()) < 1.5)
	{
		return true;
	}
	return false;
}

// DOL 0x8011965c [audited]
void cTaskCharGolvis::FallProcess()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTrans());
	if (m_bEnableGravity)
	{
		vTrans()[1] -= 0.1875f;
		v[1] += 10.5f;
		float _discard73;
		Constants::sFace* _discard74;
		cTaskBgHit* _discard75;
		if (!Constants::PACN_APP()->pTaskCollision()->CheckBottom(v, _discard73, v, _discard74, _discard75) || v[1] + 0.1875f < vTrans()[1])
		{
			m_bOnFloor = false;
			return;
		}
		vTrans()[1] = v[1];
		m_bOnFloor = true;
	}
}

// DOL 0x80119738 [audited]
bool cTaskCharGolvis::IsOnFloor()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTrans());
	v[1] += 10.5f;
	float _discard76;
	Constants::sFace* _discard77;
	cTaskBgHit* _discard78;
	if (!Constants::PACN_APP()->pTaskCollision()->CheckBottom(v, _discard76, v, _discard77, _discard78) || v[1] + 0.1875f < vTrans()[1])
	{
		m_bOnFloor = false;
		return false;
	}
	m_bOnFloor = true;
	return true;
}

// DOL 0x801197fc [audited: gap: the port zeroes pOut first, the DOL leaves it unchanged on failure]
bool cTaskCharGolvis::GetFloorHeight(float& pOut, Vector4 vPos)
{
	pOut = 0.0f;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vPos);
	v[1] += 10.5f;
	float _discard79;
	Constants::sFace* _discard80;
	cTaskBgHit* _discard81;
	if (Constants::PACN_APP()->pTaskCollision()->CheckBottom(v, _discard79, v, _discard80, _discard81))
	{
		pOut = v[1];
		return true;
	}
	return false;
}

// DOL 0x801198c0 [audited: gap: same as GetFloorHeight]
bool cTaskCharGolvis::GetFloor(Vector4& pOut, Vector4 vPos)
{
	pOut = Vector4();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vPos);
	v[1] += 10.5f;
	float _discard82;
	Constants::sFace* _discard83;
	cTaskBgHit* _discard84;
	if (Constants::PACN_APP()->pTaskCollision()->CheckBottom(v, _discard82, v, _discard83, _discard84))
	{
		::Constants::jetGeoVecCopy(pOut, v);
		return true;
	}
	return false;
}

// DOL 0x80119994 [audited]
void cTaskCharGolvis::AttackLand(int nKind)
{
	CustomKingdomUtil::Assert(nKind >= 100, "min");
	CustomKingdomUtil::Assert(nKind <= 125, "min");
	if (m_pAttackLand != nullptr)
	{
		m_pAttackLand->SetAttackKind(nKind - 100);
	}
}

// DOL 0x801199b8 [audited]
void cTaskCharGolvis::AttackLandRegist(cTaskBgGimmick* pGimmick)
{
	m_pAttackLand = new cTaskLandAttack(this, pGimmick);
}

// DOL 0x80119a0c [audited]
bool cTaskCharGolvis::AttackLandIsEnd()
{
	if (m_pAttackLand != nullptr)
	{
		return m_pAttackLand->IsAttackEnd();
	}
	return true;
}

// DOL 0x80119a30 [audited]
void cTaskCharGolvis::AttackLandPause(bool nFlag)
{
	if (m_pAttackLand != nullptr)
	{
		m_pAttackLand->Pause(nFlag);
	}
}

// DOL 0x80119a50 [audited]
bool cTaskCharGolvis::AttackLandGetMsg(uint& pMes)
{
	if (m_pAttackLand != nullptr)
	{
		return m_pAttackLand->GetMsg(pMes);
	}
	return false;
}

// DOL 0x80119a74 [audited]
uint cTaskCharGolvis::AttackName2ID(string pName)
{
	for (uint num = 0u; (long)num < (long)::Constants::ARRAY_SIZE(s_aAttackName); num++)
	{
		if (CustomKingdomUtil::strcmp(s_aAttackName[num]->m_pName, pName) == 0)
		{
			return (uint)s_aAttackName[num]->m_nID;
		}
	}
	CustomKingdomUtil::Assert("illegal name");
	return 0u;
}

}
