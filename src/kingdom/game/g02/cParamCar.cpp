#include "kingdom/game/g02/cParamCar.hpp"

#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float SEC = 60.0f;              // FLOAT_8047aa38 / FLOAT_8047a9cc / FLOAT_8047a96c
constexpr float DEG = 0.017453292f;       // FLOAT_8047aa24
constexpr float MS_TO_FRAME = 6000.0f / 100000.0f;   // FUN_8000d7d4 (FLOAT_80479c38 / FLOAT_80479c3c)
}

// DOL 0x8007e358 [audited] (cTaskUnit::Init, the cParamCar block +0x2d4)
bool cParamCar::Load(int nMultiPattern)
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_CAR")) return false;   // 0x58739259
	for (int r = 0; r < 0x3d; r++) m_fRow[r] = csv.F32(r, 1, 0.0f);
	auto f = [&](int r) { return m_fRow[r]; };
	auto s = [&](int r) { return csv.S32(r, 1, 0); };
	fSpeedBase = f(5) * f(0) / SEC;
	fSpeedEmpty = f(6) / SEC;
	fSpeedBrake = f(7) / SEC;
	fSpeedNitro = f(8) / SEC;
	fSpeedOil = f(0x12) / SEC;
	fSpeedSpike = f(0x13) / SEC;
	fSpeedAdd = f(9) / SEC;
	fSpeedSub = f(10) / SEC;
	fRotAdd = DEG * (f(0xb) / SEC);
	fNitroTime = SEC * f(0xc);
	nSmoke = s(0x10);
	fSmokeTime = SEC * f(0x11);
	fSpinTimeSmoke = SEC * f(0x14);
	fSpinTimeClash = SEC * f(0x15);
	fFuelBurst = SEC * f(0x16);
	fChameleon = SEC * f(0x17);
	fMudTime0 = SEC * f(0x18);
	fMudTime1 = SEC * f(0x19);
	nMud = s(0x1a);
	nMudBig = s(0x1b);
	fSndInterior = f(0x1c);
	fSndUnitDist = f(0x1d);
	fSndMaxVolDist = f(0x1e);
	fListenerRange = f(0x24);
	fSpinRotSpeed = f(0x1f) / SEC;
	fStartWait = SEC * f(0x20);
	fBlendNormal = f(0x21) / SEC;
	fBlendLeft = f(0x22) / SEC;
	fBlendRight = f(0x23) / SEC;
	fVisibleTime = SEC * f(0x25);
	fSpikeY = f(0x26);
	fSandNodePlus = f(0x27);
	fMiiScale = 0.01f * f(0x28);   // FLOAT_8047aa3c
	fBlinkStability = SEC * f(0x2c);
	fBlinkChange = SEC * f(0x2d);
	for (int i = 0; i < 4; i++) fRelocation[i] = MS_TO_FRAME * (float)s(0x39 + i);
	if (nMultiPattern < 0)   // single player (FUN_800c5ca0 == 0)
	{
		fFuelCost = f(0xd) / SEC;
		fFuelCostNitro = f(0xe) / SEC;
		fFuelNitroStart = f(0xf);
	}
	else if (nMultiPattern == 0 || nMultiPattern == 1 || nMultiPattern == 2)   // the sys' +0x04 (0x800c5990): patterns A / B / C
	{
		static const int ROW[3] = {0x29, 0x33, 0x36};
		const int r = ROW[nMultiPattern];
		fFuelCost = f(r) / SEC;
		fFuelCostNitro = f(r + 1) / SEC;
		fFuelNitroStart = f(r + 2);
	}
	if (fMudTime0 < 13.0f) fMudTime0 = 13.0f;   // FLOAT_8047aa40
	nArmorDropFlag = s(0x2e);
	nSmokeDropFlag = s(0x31);
	fDirectionMax = f(0x2f);
	fDirectionSpeed = f(0x30) / SEC;
	fBlendCtrl = f(0x32) / SEC;
	return true;
}

// DOL 0x80087c1c [audited]
void cParamCar::SetSpeedLevel(int level)
{
	const int r = level - 1;
	fSpeedBase = m_fRow[5] * (r >= 0 && r < 0x3d ? m_fRow[r] : 0.0f) / SEC;
}

// DOL 0x8007a2e8 [audited] (cTaskPlayer::Init, the cParamCarPlayer block +0x9c8)
bool cParamCarPlayer::Load()
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_CARPLAYER")) return false;   // 0xf383e0d6
	fPadTurn = csv.F32(0, 1, 0.0f);
	fPadLeft = csv.F32(1, 1, 0.0f);
	fPadRight = csv.F32(2, 1, 0.0f);
	fPadWait = SEC * csv.F32(3, 1, 0.0f);
	fPadWaitTurn = SEC * csv.F32(4, 1, 0.0f);
	return true;
}

// DOL 0x8006d500 [audited] (cTaskEnemy::Init, the cParamCarEnemy block +0xa20)
bool cParamCarEnemy::Load()
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_CARENEMY")) return false;   // 0x0a541dd9
	nSearchRange = csv.S32(0, 1, 0);
	nTargetNear = csv.S32(1, 1, 0);
	nTargetFar = csv.S32(2, 1, 0);
	fRangeInTime = SEC * csv.F32(3, 1, 0.0f);
	fRandStability = SEC * csv.F32(4, 1, 0.0f);
	fRandChange = SEC * csv.F32(5, 1, 0.0f);
	fCamRandStability = SEC * csv.F32(6, 1, 0.0f);
	fCamRandChange = SEC * csv.F32(7, 1, 0.0f);
	nSpinScore = csv.S32(8, 1, 0);
	nSearchChameleon = csv.S32(9, 1, 0);
	nSearchRed = csv.S32(10, 1, 0);
	fRedRangeInTime = SEC * csv.F32(11, 1, 0.0f);
	fNitroUseTime = SEC * csv.F32(12, 1, 0.0f);
	return true;
}

}
