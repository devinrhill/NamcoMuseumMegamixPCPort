#include "kingdom/game/g02/cTaskConsParts.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"

// Rally-X Remix (g02) single-player console parts. Ported from main.dol (no C# exists for Rally-X). Every part ends its Execute with the base
// cTaskRallyAnitaBase::Execute (0x80122f3c: the layouts step unless frozen); cTaskConsResult / cTaskConsLose return early when frozen.
// "Animate" = anita state bit 2, "Show" = bit 1; the constructors register the anitas with bit 2 cleared (frozen on frame 0, visible).
namespace kingdom::game::g02
{

namespace
{
// the DOL passes the CRCs 0x9d901d89 / 0x7bc6cf12 / 0x3a24042c
const char* const CAR01 = "ANITA_CAR01";
const char* const RESULT = "ANITA_RESULT";
const char* const RADAR = "ANITA_RADAR";

// 0x8036bbb0 / 0x8036bbe0 / 0x8036bc10: the fuel gauge's level clips (down / up / steady) by level 0..11
constexpr int FUEL_DOWN[12] = {0xc5, 0xc9, 0xcd, 0xd1, 0xd5, 0xd9, 0xdd, 0xe1, 0xe5, 0xe9, 0xed, 0xf1};
constexpr int FUEL_UP[12] = {0xc3, 0xc7, 0xcb, 0xcf, 0xd3, 0xd7, 0xdb, 0xdf, 0xe3, 0xe7, 0xeb, 0xef};
constexpr int FUEL_LEVEL[12] = {0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec, 0xf0};
// 0x80476d48 / 0x8036bc40: single-player layer patterns of the fuel anitas (0..2: [player index], 3..8: words 3..8). The tables of the
// 2 / 3 / 4 player layouts (0x80476d50 + 0x8047a8e8, 0x80390f48 + 0x8036bcf4, 0x80390f58 + 0x8036bd00) are NOT PORTED (multiplayer).
constexpr int FUEL_PAT_1P_GAUGE = 4;
constexpr int FUEL_PAT_1P[6] = {5, 6, 7, 8, 9, 10};

constexpr int DIGIT_SMALL[10] = {0x01, 0x04, 0x07, 0x0a, 0x0d, 0x10, 0x13, 0x16, 0x19, 0x1c};   // 0x8036b950 (flags / rest)
constexpr int DIGIT_SCORE[10] = {0x25, 0x28, 0x2b, 0x2e, 0x31, 0x34, 0x37, 0x3a, 0x3d, 0x40};   // 0x8036b978
constexpr int DIGIT_STAGE[10] = {0x24, 0x27, 0x2a, 0x2d, 0x30, 0x33, 0x36, 0x39, 0x3c, 0x3f};   // 0x8036b9a0
constexpr int INFO_ICON[5] = {0x000, 0x092, 0x108, 0x10b, 0x10e};                            // 0x8036b9c8 [player count]
constexpr int DIGIT_TIME[10] = {0x46, 0x49, 0x4c, 0x4f, 0x52, 0x55, 0x58, 0x5b, 0x5e, 0x61};    // 0x8036bb20
constexpr int TIME_PAT[5] = {0, 0x40, 0x41, 0x42, 0x43};                                       // 0x8036bb48 [player count]
constexpr uint TIME_ORDER[8] = {7, 6, 5, 4, 3, 2, 1, 0};                                     // 0x8036bb60: anita on layer i
constexpr int MESSAGE_ANIM[4] = {0xbc, 0xb3, 0xb6, 0xb9};                                    // 0x8036bd10 (every third word)
constexpr u32 RESULT_DIGIT[10] = {0x00, 0x03, 0x06, 0x09, 0x0c, 0x0f, 0x12, 0x15, 0x18, 0x1b};  // 0x8036bd70
constexpr u32 RESULT_COUNT[10] = {0x01, 0x04, 0x07, 0x0a, 0x0d, 0x10, 0x13, 0x16, 0x19, 0x1c};  // 0x8036bd98

constexpr float SIGNAL_COUNT[5] = {120.0f, 180.0f, 240.0f, 300.0f, 360.0f};   // FLOAT_8047a930 .. 8047a940

int Digit(int n, int pow10)
{
	return (n / pow10) % 10;   // the DOL's signed divide sequence
}

bool IsBoss()
{
	return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsBossStage();   // 0x800c5ba8
}

// 0x800c53ec: count a frame timer up to `limit`; true (clamped) once reached
bool CountUp(float& t, float limit)
{
	t += 1.0f;   // FLOAT_8047b2e4
	if (limit <= t)
	{
		t = limit;
		return true;
	}
	return false;
}

int FuelLevel(float fuel)
{
	int n = (int)(12.0f * (fuel / 100.0f));   // FLOAT_8047a928 / FLOAT_8047a92c
	if (n == 0xc) n = 0xb;
	return n;
}
}

// ======================================================================== cTaskConsFuel

// DOL 0x8006210c
cTaskConsFuel::cTaskConsFuel(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsFuel");
	nDrawPriority(0x80000001u);
	m_nMode = 0;
	m_fFuel = 0.0f;
	Regist(0, CAR01, 0xf4);
	Animate(0, false);
	static const int ANIM[8] = {0xf2, 0xf2, 0xa7, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
	for (uint i = 1; i < 9; i++)
	{
		Regist(i, CAR01, ANIM[i - 1]);
		pAnita(i)->SetState(3u, false);
	}
	SetPosLayer(0, 3, 0);
	SetPosLayer(1, 3, 0);
	SetPosLayer(2, 3, 0);
	SetPosLayer(3, 0, 0);
	SetPosLayer(4, 0, 0);
	SetPosLayer(5, 0, 0);
	SetPosLayer(6, 5, 0);
	SetPosLayer(7, 6, 0);
	SetPosLayer(8, 7, 0);
}

// the fuel half of DOL 0x80065db0 (single player: player index 0, the 1-player layer tables)
void cTaskConsFuel::SetCar(kingdom::cTaskModel* pCar, int nPlayer)
{
	m_pCar = pCar;
	m_nPlayer = nPlayer;
	if (m_pConsole == nullptr || m_pConsole->nPlayer() != 1) return;   // NOT PORTED: the 2..4 player layouts
	for (uint i = 0; i < 3; i++) SetPosLayer(i, FUEL_PAT_1P_GAUGE, 0);
	for (uint i = 0; i < 6; i++) SetPosLayer(3 + i, FUEL_PAT_1P[i], 0);
}

// DOL 0x8006957c
bool cTaskConsFuel::Execute(cJetTask* pParam)
{
	if (m_pCar != nullptr)
	{
		sConsCar car;
		cTaskConsole::CarState(m_pCar, car);
		switch (m_nMode)
		{
		case 0:
			m_nMode = 1;
			break;
		case 1:
			if (1 < m_pConsole->nState())
			{
				Animate(0, true);
				if (!IsBoss()) pAnita(3)->SetState(3u, true);
				m_fFuel = car.fFuel;
				m_nLevel = FuelLevel(car.fFuel);
				m_nMode++;
			}
			break;
		case 2:
		{
			if (IsEnd(0)) pAnita(1)->SetState(3u, true);
			if (!IsBoss() && IsEnd(3))
			{
				for (uint i = 4; i < 9; i++) pAnita(i)->SetState(3u, true);
			}
			bool next = false;
			if (!IsBoss())
			{
				if (IsEnd(1) && IsEnd(4)) next = true;
			}
			else if (IsEnd(1))
			{
				next = true;
			}
			if (next) m_nMode++;
			break;
		}
		case 3:
		{
			SetAnim(0, car.fTimer2bc > 0.0f ? 0xf6 : 0xf5);   // 0x80088458
			pAnita(2)->SetState(3u, car.fNitro > 0.0f);       // +0x2a4
			m_fFuel = car.fFuel;
			const int level = FuelLevel(car.fFuel);
			SetAnim(2, FUEL_LEVEL[level]);
			if (m_fFuel == 0.0f)
			{
				Show(1, false);
				Show(2, false);
				m_nLevel = 0;
			}
			else if (level < m_nLevel)
			{
				Show(1, true);
				SetAnim(1, FUEL_DOWN[m_nLevel]);
			}
			else if (m_nLevel < level)
			{
				Show(1, true);
				SetAnim(1, FUEL_UP[level]);
			}
			m_nLevel = level;
			if (m_nSmoke != car.nSmoke)   // +0x2cc
			{
				m_nSmoke = car.nSmoke;
				for (uint i = 0; i < 5; i++) SetAnim(i + 4, i < (uint)m_nSmoke ? 0xaa : 0xac);
			}
			break;
		}
		default:
			break;
		}
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsInfo

// DOL 0x800619d8
cTaskConsInfo::cTaskConsInfo(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsInfo");
	nDrawPriority(0x80000001u);
	int n = cTaskConsole::SysPlayerNum();
	int icon = (n >= 0 && n < 5) ? INFO_ICON[n] : 0;
	if (IsBoss()) icon = 0x119;
	static const int ANIM[17] = {0, 0xad, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0xa3, 0x21, 0, 0, 0, 0, 0x95, 0x1e, 0};
	for (uint i = 0; i < 17; i++)
	{
		Regist(i, CAR01, i == 0 ? icon : ANIM[i]);
		Animate(i, false);
	}
	SetPosLayer(1, 2, 0);
	for (uint i = 0; i < 6; i++) SetPosLayer(2 + i, 0x39 + (int)i, 0);
	SetPosLayer(8, 1, 0);
	SetPosLayer(10, 0x47, 6);
	SetPosLayer(0xb, 0x47, 5);
	SetPosLayer(9, 0x47, 4);
	SetPosLayer(0xc, 0x47, 3);
	SetPosLayer(0xd, 0x47, 2);
	SetPosLayer(0xe, 0, 0);
	SetPosLayer(0xf, 0x47, 1);
	SetPosLayer(0x10, 0x47, 0);
	if (IsBoss())
	{
		for (uint i = 1; i < 8; i++) Show(i, false);
		Show(0xe, false);
		Show(0xf, false);
		Show(0x10, false);
	}
}

// DOL 0x8006aa78
bool cTaskConsInfo::Execute(cJetTask* pParam)
{
	switch (m_nMode)
	{
	case 0:
		if (1 < m_pConsole->nState())
		{
			Animate(0, true);
			m_nScore = m_pConsole->nScore();
			const int total = m_pConsole->nFlagTotal();   // console +0x1c0
			const int hi = DIGIT_SMALL[Digit(total, 10)], lo = DIGIT_SMALL[Digit(total, 1)];
			pAnita(0xc)->SetAnimNo(hi, true);
			pAnita(0xd)->SetAnimNo(lo, true);
			if (IsBoss())
			{
				pAnita(10)->SetAnimNo(hi, true);
				pAnita(0xb)->SetAnimNo(lo, true);
			}
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0))
		{
			for (uint i = 1; i < 17; i++) Animate(i, true);
			m_nMode++;
		}
		break;
	case 2:
	{
		const int score = m_pConsole->nScore();   // app +0x300
		if (m_nScore != score)
		{
			m_nScore = score;
			static const int POW[6] = {100000, 10000, 1000, 100, 10, 1};
			for (uint i = 0; i < 6; i++) pAnita(7 - i)->SetAnimNo(DIGIT_SCORE[Digit(score, POW[i])], true);
		}
		sConsCar car;
		cTaskConsole::CarState(m_pCar, car);
		int flags = car.nFlags;   // +0x2d0
		if (m_nFlags != flags)
		{
			m_nFlags = flags;
			if (IsBoss()) flags = m_pConsole->nFlagTotal() - m_nFlags;   // the RED car: flags left
			pAnita(10)->SetAnimNo(DIGIT_SMALL[Digit(flags, 10)], true);
			pAnita(0xb)->SetAnimNo(DIGIT_SMALL[Digit(flags, 1)], true);
		}
		int rest = m_pConsole->nRest();   // app +0x32c, used as an index without a range check in the DOL
		if (rest < 0) rest = 0;
		if (rest > 9) rest = 9;           // UNVERIFIED clamp (the DOL would read the next tables)
		pAnita(0x10)->SetAnimNo(DIGIT_SMALL[rest], true);
		if (car.nCrash != 0)                SetAnim(0xe, 0x9a);   // +0x260
		else if (car.fTimer2c0 > 0.0f)      SetAnim(0xe, 0x98);   // 0x80088498
		else if (car.n2c8 == 1)             SetAnim(0xe, 0x97);   // 0x800884c8
		else if (car.fNitro > 0.0f)         SetAnim(0xe, 0x9d);
		else                                SetAnim(0xe, 0x96);
		break;
	}
	default:
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsSignal

// DOL 0x80062530
cTaskConsSignal::cTaskConsSignal(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsSignal");
	m_nMode = 0;
	nDrawPriority(0x80000003u);
	Regist(0, CAR01, 0x87);
	Animate(0, false);
}

// DOL 0x8006b234 (the lamps follow the app's start count +0x31c; m02se_14Signal1 for the red lamps, m02se_15Signal2 for the last)
bool cTaskConsSignal::Execute(cJetTask* pParam)
{
	if (1 < m_pConsole->nState())
	{
		const float count = m_pConsole->fCount();
		switch (m_nMode)
		{
		case 0:
			Animate(0, true);
			m_nMode++;
			break;
		case 1:
			if (IsEnd(0)) m_nMode++;
			break;
		case 2:
		{
			const int anim = pAnita(0)->GetAnimNo();
			if (anim == 0x87)
			{
				if (SIGNAL_COUNT[0] <= count)
				{
					cTaskConsole::PlaySE("m02se_14Signal1");   // DAT_8047a8dc
					pAnita(0)->SetAnimNo(0x89, true);
				}
			}
			else if (anim == 0x89)
			{
				if (SIGNAL_COUNT[1] <= count)
				{
					cTaskConsole::PlaySE("m02se_14Signal1");
					pAnita(0)->SetAnimNo(0x8b, true);
				}
			}
			else if (anim == 0x8b)
			{
				if (SIGNAL_COUNT[2] <= count)
				{
					cTaskConsole::PlaySE("m02se_14Signal1");
					pAnita(0)->SetAnimNo(0x8d, true);
				}
			}
			else if (anim == 0x8d && SIGNAL_COUNT[3] <= count)
			{
				cTaskConsole::PlaySE("m02se_15Signal2");   // DAT_8047a8e0
				pAnita(0)->SetAnimNo(0x8f, true);
				m_nMode++;
			}
			break;
		}
		case 3:
			if (SIGNAL_COUNT[4] <= count)
			{
				pAnita(0)->SetAnimNo(0x91, true);
				m_nMode++;
			}
			break;
		default:
			break;
		}
		if (m_pConsole->nState() == 3) Show(0, false);   // the retry after a miss starts without the lamp
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsTime

// DOL 0x8006272c
cTaskConsTime::cTaskConsTime(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsTime");
	nDrawPriority(0x80000001u);
	m_nMode = 0;
	Regist(2, CAR01, 0x63);
	Animate(2, false);
	Regist(5, CAR01, 0x114);
	Animate(5, false);
	static const uint DIGITS[6] = {0, 1, 3, 4, 6, 7};
	for (uint i : DIGITS)
	{
		Regist(i, CAR01, 0x45);
		Animate(i, false);
	}
	const int n = m_pConsole != nullptr ? m_pConsole->nPlayer() : 1;
	const int pat = (n >= 0 && n < 5) ? TIME_PAT[n] : 0;
	for (uint i = 0; i < 8; i++) SetPosLayer(TIME_ORDER[i], pat, i);
}

// DOL 0x80069c08 (the play time app +0x318 in milliseconds; minutes 0x800c59e8, seconds 0x800c59bc, milliseconds 0x800c59a0)
bool cTaskConsTime::Execute(cJetTask* pParam)
{
	switch (m_nMode)
	{
	case 0:
		if (1 < m_pConsole->nState())
		{
			static const uint ORDER[8] = {2, 5, 0, 1, 3, 4, 6, 7};
			for (uint i : ORDER) Animate(i, true);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(2)) m_nMode++;
		break;
	case 2:
	{
		const float f = m_pConsole->fPlayTime();
		const u32 t = f > 0.0f ? (u32)f : 0u;   // __cvt_fp2unsigned
		const u32 ms = t % 1000, sec = (t / 1000) % 60, min = (t / 60000) % 60;
		pAnita(0)->SetAnimNo(DIGIT_TIME[min / 10], true);
		pAnita(1)->SetAnimNo(DIGIT_TIME[min % 10], true);
		pAnita(3)->SetAnimNo(DIGIT_TIME[sec / 10], true);
		pAnita(4)->SetAnimNo(DIGIT_TIME[sec % 10], true);
		pAnita(6)->SetAnimNo(DIGIT_TIME[(ms / 10) / 10], true);
		pAnita(7)->SetAnimNo(DIGIT_TIME[(ms / 10) % 10], true);
		break;
	}
	default:
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsStage

// DOL 0x80062afc
cTaskConsStage::cTaskConsStage(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsStage");
	nDrawPriority(0x80000001u);
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const uint world = sys != nullptr ? (uint)sys->nWorld() : 0u, stage = sys != nullptr ? (uint)sys->nStageInWorld() : 0u;
	Regist(0, CAR01, DIGIT_STAGE[world % 10]);
	Animate(0, false);
	Regist(1, CAR01, DIGIT_STAGE[stage % 10]);
	Animate(1, false);
	Regist(2, CAR01, 0x42);
	Animate(2, false);
	SetPosLayer(0, 0x45, 0);
	SetPosLayer(1, 0x45, 0);
	SetPosLayer(2, 0x45, 0);
	pAnita(0)->vTrans()[0] -= 32.0f;   // FLOAT_8047a8f4
	pAnita(2)->vTrans()[0] -= 16.0f;   // FLOAT_8047a8f8
}

// DOL 0x80069aa4
bool cTaskConsStage::Execute(cJetTask* pParam)
{
	if (1 < m_pConsole->nState())
	{
		Animate(0, true);
		Animate(1, true);
		Animate(2, true);
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsMessage

// DOL 0x80062e14 (CSV_FLAG through a temporary cParamFlagData: column 2 of row stage + (world - 1) * 7)
cTaskConsMessage::cTaskConsMessage(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsMessage");
	nDrawPriority(0x80000004u);
	m_nMode = 0;
	Regist(0, CAR01, 0xbf);
	Animate(0, false);
	Regist(1, CAR01, 0xb0);
	Animate(1, false);
	Regist(2, CAR01, 0xbc);
	Animate(2, false);
	SetPosLayer(1, 3, 0);
	uint v = 0;
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	kingdom::cKingdomCSV csv;
	if (sys != nullptr && csv.RegistData("CSV_FLAG"))
	{
		const int row = sys->nStageInWorld() + (sys->nWorld() - 1) * 7;
		if (row < 0) v = 0;
		else if (row < 0x1c) v = (uint)csv.S32(row, 2, 0);
	}
	if (v < 4)
	{
		pAnita(2)->SetAnimNo(MESSAGE_ANIM[v], true);
	}
	else
	{
		Show(0, false);
		Show(1, false);
		Show(2, false);
	}
}

// DOL 0x80069380
bool cTaskConsMessage::Execute(cJetTask* pParam)
{
	switch (m_nMode)
	{
	case 0:
		if (1 < m_pConsole->nState())
		{
			Animate(0, true);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0))
		{
			Animate(1, true);
			Animate(2, true);
			m_nMode++;
		}
		break;
	case 2:
		if (IsEnd(1))
		{
			pAnita(1)->SetAnimNo(0xb1, true);
			m_nMode++;
		}
		break;
	default:
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsResult

// DOL 0x80064008
cTaskConsResult::cTaskConsResult(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsResult");
	nDrawPriority(0x80000005u);
	m_fTimer = 0.0f;
	m_nMode = 0;
	static const int ANIM[11] = {0x3c, 0x39, 0x36, 0x1e, 0x27, 0, 0, 0, 0, 0, 0};
	for (uint i = 0; i < 11; i++)
	{
		Regist(i, RESULT, ANIM[i]);
		Animate(i, false);
	}
	for (uint i = 0; i < 6; i++) SetPosLayer(i + 5, 0, 5 - i);
}

// the six score digits (anitas 5 .. 10 = 100000s .. 1s) of Execute
void cTaskConsResult::SetScore(const u32* table)
{
	const int score = m_pConsole->nScore();
	static const int POW[6] = {100000, 10000, 1000, 100, 10, 1};
	for (uint i = 0; i < 6; i++) pAnita(5 + i)->SetAnimNo((int)table[Digit(score, POW[i])], true);
}

// DOL 0x80068220 (stage clear: app state 7; the app polls IsResultEnd = mode 3, which lasts one frame)
bool cTaskConsResult::Execute(cJetTask* pParam)
{
	if (m_pConsole->IsFrozen()) return true;
	switch (m_nMode)
	{
	case 0:
		if (m_pConsole->nState() == 7 && m_pConsole->nStep() == 2)
		{
			Animate(0, true);
			Animate(3, true);
			if (!IsBoss()) SetScore(RESULT_DIGIT);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0) && IsEnd(3))
		{
			m_nMode++;
			m_fTimer = 0.0f;
		}
		break;
	case 2:
		if (CountUp(m_fTimer, m_pConsole->fClearFlagWait())) m_nMode++;   // app +0x280 +0x38
		break;
	case 3:
		if (!IsBoss())
		{
			Animate(4, true);
			for (uint i = 0; i < 6; i++) Animate(i + 5, true);
		}
		m_nMode++;
		break;
	case 4:
		if (!IsBoss())
		{
			if (m_pConsole->nStep() == 5)
			{
				cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
				const int hi = cTaskConsole::StageHighScore(sys != nullptr ? sys->nStageIdx() : 0);   // 0x800c62a8(record, sys +8)
				if (m_pConsole->nScore() < hi)
				{
					for (uint i = 0; i < 6; i++) Animate(i + 5, false);
				}
				else
				{
					for (uint i = 0; i < 6; i++) Animate(i + 5, true);
					cTaskConsole::PlaySE("m00se_06Hiscore");   // DAT_8047a8d8
				}
				SetAnim(0, 0x3e);
				SetAnim(3, 0x20);
				m_nMode++;
			}
			SetScore(RESULT_COUNT);   // every frame of mode 4 (the score counts up from the fuel), also on the frame that leaves it
		}
		else
		{
			SetAnim(0, 0x3e);
			SetAnim(3, 0x20);
			m_nMode++;
		}
		break;
	default:
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsLose

// DOL 0x800643e4
cTaskConsLose::cTaskConsLose(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsLose");
	nDrawPriority(0x80000005u);
	m_nMode = 0;
	m_fTimer = 0.0f;
	Regist(0, RESULT, 0);
	Show(0, false);
	Regist(1, RESULT, 0);
	Show(1, false);
}

// DOL 0x80066da8 (boss stage lost: app state 10)
bool cTaskConsLose::Execute(cJetTask* pParam)
{
	if (m_pConsole->IsFrozen()) return true;
	switch (m_nMode)
	{
	case 0:
		if (m_pConsole->nState() == 10)
		{
			pAnita(0)->SetAnimNo(0x3c, true);
			Show(0, true);
			pAnita(1)->SetAnimNo(0x24, true);
			Show(1, true);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0) && IsEnd(1))
		{
			pAnita(0)->SetAnimNo(0x3d, true);
			pAnita(1)->SetAnimNo(0x25, true);
			m_nMode = 2;
			m_fTimer = 0.0f;
		}
		break;
	case 2:
		if (CountUp(m_fTimer, m_pConsole->fLoseFlagWait()))   // app +0x280 +0x44
		{
			pAnita(1)->SetAnimNo(0x26, true);
			m_nMode++;
		}
		break;
	case 3:
		m_nMode = 4;
		break;
	case 4:
		pAnita(0)->SetAnimNo(0x3e, true);
		m_nMode++;
		break;
	case 5:
		if (IsEnd(0) && IsEnd(1)) m_nMode = 7;
		break;
	default:
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ======================================================================== cTaskConsBang

// DOL 0x80064610
cTaskConsBang::cTaskConsBang(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsBang");
	nDrawPriority(0x80000005u);
	Regist(0, CAR01, 0);
	cTaskRallyAnitaBase::Show(0, false);
}

// DOL 0x80066a74
bool cTaskConsBang::Execute(cJetTask* pParam)
{
	(void)pParam;
	if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0)
	{
		if (!m_pConsole->IsFrozen()) AnitaVector->UpdateFrame();   // 0x80014d58
	}
	return true;
}

// the body of DOL 0x8006662c
void cTaskConsBang::Show()
{
	pAnita(0)->SetAnimNo(0x118, true);
	cTaskRallyAnitaBase::Show(0, true);
}

// the body of DOL 0x8006669c / 0x800667c4
void cTaskConsBang::Hide()
{
	cTaskRallyAnitaBase::Show(0, false);
}

// ======================================================================== cTaskConsWarning

// DOL 0x80064814 (registered animating, all hidden)
cTaskConsWarning::cTaskConsWarning(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsWarning");
	nDrawPriority(0x80000005u);
	Regist(0, RADAR, 0);
	Regist(3, RADAR, 0x58);
	Regist(2, RADAR, 0x59);
	Regist(4, RADAR, 0x5a);
	Regist(1, RADAR, 0x5b);
	HideAll();
	m_bActive = false;
}

void cTaskConsWarning::HideAll()
{
	for (uint i = 0; i < 5; i++) cTaskRallyAnitaBase::Show(i, false);
}

// DOL 0x800668cc
bool cTaskConsWarning::Execute(cJetTask* pParam)
{
	if (m_pConsole->nState() != 4 && m_bActive)
	{
		m_bActive = false;
		HideAll();
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

}
