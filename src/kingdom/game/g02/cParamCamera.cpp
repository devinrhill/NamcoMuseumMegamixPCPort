#include "kingdom/game/g02/cParamCamera.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047c01c
constexpr float FPS = 60.0f;              // FLOAT_8047c020
}

// the construction inlined in cCamera::Init: cJetCSV ctor (FUN_801393c0), the cParamCamera vtable, then its slot +0x08 (Clear)
cParamCamera::cParamCamera()
{
	Init();
}

// DOL 0x801281a0 (the empty Term 0x80128234, then the cJetCSV base)
cParamCamera::~cParamCamera()
{
	Term();
}

// DOL 0x801280b0: memset +0x20 0xc0 (the six entries), then 0 into the scalars; +0xf4 / +0xf8 / +0x104 / +0x108 are left alone
void cParamCamera::Init()
{
	for (Entry& e : m_Entry) e = Entry{};
	m_fPosYLimit = m_fRotYSpeed = m_fRotYBetween = m_fRangeBrake = m_fRangeNitro = 0.0f;
	m_f2PRangeBrake = m_f2PRangeNitro = 0.0f;
	m_fToGamePosSpd = m_fToGamePosBetween = m_fToGameTgtSpd = m_fToGameTgtBetween = 0.0f;
	m_fFirstRotY = m_fFirstRotYSpeed = m_fFirstRotYBetween = m_fFirstRangeStart = m_fFirstRangeEnd = 0.0f;
	m_fSpdMax = m_fSpdCurveMax = m_fSpdBetween = m_fSpdCurveBetween = m_fClearRotSpeedY = 0.0f;
	m_bLoaded = false;
}

// DOL 0x80128234
void cParamCamera::Term()
{
}

// the CSV_CAMERA reads inlined in cCamera::Init: cKingdomSys data lookup (FUN_8000bcd4, CRC 0xe12a4259) -> cJetCSV SetData (FUN_80139428), then
// F32(row, column 1, default 0) in this order. m_bLoaded is set whether or not the data was found (as the DOL: it does not test SetData).
void cParamCamera::Load()
{
	RegistData("CSV_CAMERA");
	auto f = [this](int row) { return F32(row, 1, 0.0f); };   // an rvalue default (an lvalue picks the "out" overload)
	auto entry = [&](Entry& e, int x, int rotx, int range, int fovy) {
		e.vTarget[0] = f(x);
		e.vTarget[1] = f(x + 1);
		e.vTarget[2] = f(x + 2);
		e.fRotX = DEG2RAD * f(rotx);
		e.fRange = f(range);
		e.fFovy = f(fovy);
	};
	// in the DOL's read order (kept for the audit)
	entry(m_Entry[eENTRY_QUARTER], 0x00, 0x03, 0x07, 0x0c);
	m_fPosYLimit = f(0x0b);
	m_fRotYSpeed = f(0x04);
	m_fRotYBetween = f(0x05);
	m_fRangeBrake = f(0x06);
	m_fRangeNitro = f(0x08);
	m_fRangeSpeed = f(0x09);
	m_fRangeBetween = f(0x0a);
	entry(m_Entry[eENTRY_QUARTER_2P], 0x0e, 0x11, 0x15, 0x1a);
	m_f2PRangeBrake = f(0x14);
	m_f2PRangeNitro = f(0x16);
	m_f2PRangeSpeed = f(0x17);
	m_f2PRangeLimit = f(0x18);
	entry(m_Entry[eENTRY_TOP], 0x1c, 0x1f, 0x20, 0x21);
	entry(m_Entry[eENTRY_TOP_2P], 0x23, 0x26, 0x27, 0x28);
	entry(m_Entry[eENTRY_FIRST], 0x2a, 0x2d, 0x31, 0x0c);      // range = FIRST_RANGE_START, fovy = QUARTER_FOVY
	m_fFirstRotY = DEG2RAD * f(0x2e);
	m_fFirstRotYSpeed = DEG2RAD * f(0x2f);
	m_fFirstRotYBetween = DEG2RAD * f(0x30);
	m_fFirstRangeStart = f(0x31);
	m_fFirstRangeEnd = f(0x32);
	entry(m_Entry[eENTRY_FIRST_2P], 0x2a, 0x2d, 0x31, 0x0c);   // the same rows again
	m_fToGamePosSpd = f(0x37);
	m_fToGamePosBetween = f(0x38);
	m_fToGameTgtSpd = f(0x39);
	m_fToGameTgtBetween = f(0x3a);
	m_fSpdMax = f(0x41);
	m_fSpdCurveMax = f(0x42);
	m_fSpdBetween = f(0x43);
	m_fSpdCurveBetween = f(0x44);
	m_fClearRotSpeedY = f(0x46) / FPS;
	m_bLoaded = true;
}

}
