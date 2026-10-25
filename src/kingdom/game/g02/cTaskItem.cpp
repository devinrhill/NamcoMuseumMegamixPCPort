#include "kingdom/game/g02/cTaskItem.hpp"

#include <cmath>
#include <cstdio>

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float ITEM_Y = 0.04f;        // FLOAT_8047a988 (items) / FLOAT_8047afe4 (flags)
constexpr float FLY_TIME = 60.0f;      // FLOAT_8047afcc
constexpr float PI = 3.1415927f;

// FUN_803007f0 = nw4r::math::SinFIdx (256 units a turn); the DOL passes FLOAT_8047afd8 * FLOAT_8047afd0 * FLOAT_8047afd4 * (t / 60) =
// (128 / pi) * (pi / 180) * 180 * t / 60 = 128 t / 60 units = pi t / 60 radians: half a sine over the flight (height 1)
float FlyHeight(float t) { return std::sin(PI * (t / FLY_TIME)); }

int Round(float v) { return (int)(v + (0.0f <= v ? 0.5f : -0.5f)); }   // FLOAT_8047afdc / FLOAT_8047afe0

void Show(kingdom::cKingdomModel* m, bool b)
{
	if (m == nullptr) return;
	if (b) m->Visible();     // FUN_8001a1f4
	else m->Invisible();     // FUN_8001a330
}

bool IsBoss() { return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsBossStage(); }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// items

// DOL 0x8007113c / 0x80071028 / 0x80070f14 / 0x80070e00 (the item part of the factory entries, after the cTaskGimmick head)
cTaskItem::cTaskItem(const char* key)
{
	SetName("g02::cTaskItem");
	SetupModel(0, key, "ITEM_ANI_OTHER", true);   // + ITEM_ANI_OTHER 0x3bff28c2, SHADOW_CIRCLE on "shadow"
}

// DOL 0x80075684 (identical: 0x80074fc0 / 0x8007489c / 0x800741d8)
bool cTaskItem::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (IsFrozen()) return true;
	kingdom::cKingdomModel* m = pModel(0);
	if (m == nullptr) return true;
	m->vTrans() = Vector4((float)(m_nX - 4), ITEM_Y, (float)(m_nZ - 8), 1.0f);
	RotY(m->vRotate());   // FUN_800b1d74
	m->UpdateFrame();
	return true;
}

// DOL 0x800752f8 (identical: 0x80074bd4 / 0x80074510 / 0x80073dec)
bool cTaskItem::OnHit(cTaskUnit* pUnit)
{
	if (IsCollector(pUnit))   // FUN_800b1dd0
	{
		FxOnce(9, pModel(0));  // FUN_80021a9c(0, tmp, 9, 1, 0, 0, model trans, model rot)
		Show(pModel(0), false);
		Disable();             // vtable +0x54
	}
	return cTaskGimmick::OnHit(pUnit);   // FUN_800b1d4c
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// cTaskItemFlag

cTaskItemFlag::cTaskItemFlag()
{
	SetName("g02::cTaskItemFlag");
	// +0x210 cKingdomSndActor (FUN_8002654c): NOT PORTED
	InitFlag();   // vtable +0x3c with the cTaskItemFlag vtable
}

// DOL 0x800b28c0
void cTaskItemFlag::InitFlag()
{
	m_bTaken = false;
	m_fTime = 0.0f;
	m_vTo[0] = m_vTo[1] = m_vTo[2] = 0.0f;
	m_vTo[3] = 1.0f;
	m_vFrom[0] = m_vFrom[1] = m_vFrom[2] = 0.0f;
	m_vFrom[3] = 1.0f;
	kingdom::cKingdomModel* m = SetupModel(0, "ITEM_FLAG", "ITEM_ANI_FLAG", true);   // 0x9cc79e8a, 0xce931d73
	// boss stage: every model's +0x290 = 1 (the push-back model's draw word, as the car's colour): NOT PORTED
	(void)IsBoss();
	Show(m, false);
	m_bFlying = false;
}

// DOL 0x800b2a20 (this, 1, key)
void cTaskItemFlag::SetupFlag(const char* key)
{
	SetupModel(1, key, "ITEM_ANI_FLAG", true);
	// boss stage: models 0 / 1 +0x290 = 1: NOT PORTED (see InitFlag)
}

// DOL 0x800b2b3c
bool cTaskItemFlag::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (IsFrozen()) return true;
	if (m_bFlying)
	{
		Vector4 p(m_vFrom[0], m_vFrom[1], m_vFrom[2], m_vFrom[3]);
		p.x = m_fTime * ((m_vTo[0] - m_vFrom[0]) / FLY_TIME) + p.x;
		p.y = p.y + FlyHeight(m_fTime);
		p.z = m_fTime * ((m_vTo[2] - m_vFrom[2]) / FLY_TIME) + p.z;
		for (uint i = 0; i < 2; i++)
			if (kingdom::cKingdomModel* m = pModel(i)) m->vTrans() = p;
		if (FLY_TIME <= m_fTime)
		{
			m_bFlying = false;
			for (uint i = 0; i < 2; i++)
				if (kingdom::cKingdomModel* m = pModel(i)) m->vTrans() = Vector4(m_vTo[0], m_vTo[1], m_vTo[2], m_vTo[3]);
			Enable();   // vtable +0x58
		}
		m_fTime += 1.0f;
	}
	for (uint i = 0; i < 2; i++)
	{
		kingdom::cKingdomModel* m = pModel(i);
		if (m == nullptr) continue;   // (the DOL does not test; every flag has both models)
		if (!m_bFlying)
		{
			m->vTrans() = Vector4((float)(m_nX - 4), 0.0f, (float)(m_nZ - 8), 1.0f);
			m->vTrans().y = ITEM_Y;
			Show(m->pExpandMdl(0), true);    // the shadow (m +0x60)
		}
		else
		{
			const Vector4 t = m->vTrans();
			m_nX = Round(t.x) + 4;
			m_nZ = Round(t.z) + 8;
			Show(m->pExpandMdl(0), false);
		}
		RotY(m->vRotate());   // FUN_800b1d74
		m->UpdateFrame();
	}
	// the sound actor follows model 1 (FUN_80026620 / FUN_80026660): NOT PORTED
	return true;
}

// DOL 0x800b2f38
bool cTaskItemFlag::OnHit(cTaskUnit* pUnit)
{
	if (IsCollector(pUnit))   // FUN_800b1dd0
	{
		FxOnce(9, pModel(0));  // FUN_80021a9c(0, tmp, 9, 1, 0, 0, model 0 trans, model 0 rot)
		HideModels();
		Disable();             // vtable +0x54
		m_bTaken = true;
		m_nType = 0xb;         // now a plain flag (the radar shows the taken icon, the flag manager may drop it again)
	}
	return cTaskGimmick::OnHit(pUnit);   // FUN_800b1d4c
}

// DOL 0x800b31e0
void cTaskItemFlag::Enable()
{
	Show(pModel(m_bTaken ? 0u : 1u), true);
	cTaskGimmick::Enable();   // FUN_800b1d60
}

// DOL 0x800b30ac
bool cTaskItemFlag::Drop(const float pTo[4], const float pFrom[4])
{
	if (IsActive() || m_bFlying) return false;
	for (int i = 0; i < 4; i++)
	{
		m_vTo[i] = pTo[i];
		m_vFrom[i] = pFrom[i];
	}
	m_fTime = 0.0f;
	if (kingdom::cKingdomModel* m = pModel(0)) m->vTrans() = Vector4(m_vFrom[0], m_vFrom[1], m_vFrom[2], m_vFrom[3]);
	m_bFlying = true;
	Show(pModel(0), true);
	Se("m02se_20Fragdrop");   // DAT_8047afc0 (no actor)
	return true;
}

// DOL 0x800b3238
void cTaskItemFlag::HideModels()
{
	for (uint i = 0; i < 2; i++) Show(pModel(i), false);
}

// DOL 0x80072ca0
cTaskItemFlagNormal::cTaskItemFlagNormal()
{
	SetName("g02::cTaskItemFlagNormal");
	SetupFlag("ITEM_FLAG");   // 0x9cc79e8a
}

// DOL 0x80076948
void cTaskItemFlagNormal::Disable()
{
	HideModels();
	cTaskGimmick::Disable();   // FUN_800b1d54
}

// DOL 0x80071250
cTaskItemFlagSpecial::cTaskItemFlagSpecial()
{
	SetName("g02::cTaskItemFlagSpecial");
	SetupFlag("ITEM_FLAG_SP");   // 0x643c81b1
}

// DOL 0x80071414
cTaskItemFlagLucky::cTaskItemFlagLucky()
{
	SetName("g02::cTaskItemFlagLucky");
	SetupFlag("ITEM_FLAG_LUCKY");   // 0x90a224ba
}

// DOL 0x80072b20 / 0x800729f4 / 0x800728c8 / 0x8007279c / 0x80072670 / 0x80072544 / 0x80072418 / 0x800722ec / 0x800721c0 / 0x80072094
// DOL 0x80071f68 / 0x80071e3c / 0x80071d10 / 0x80071be4 / 0x80071ab8 / 0x8007198c / 0x80071860 / 0x80071734 / 0x80071608 / 0x800714d8
// (numbers 01..20: the factory table 0x80391f48 entries 12..31)
cTaskItemFlagNumber::cTaskItemFlagNumber(int n)
	: m_nNumber(n < 1 ? 1 : (n > 20 ? 20 : n))
{
	SetName("g02::cTaskItemFlagNumber");
	char key[32];
	std::snprintf(key, sizeof key, "ITEM_FLAG_%02d", m_nNumber);
	SetupFlag(key);
	// the NOTGET clip (table 0x8036c478 [n]: ITEM_ANI_FLAG01_NOTGET 0xb0d35fd7 ...) into model 1's slot 0 (FUN_8001a4d8(m, 0, crc, 0, 1))
	std::snprintf(key, sizeof key, "ITEM_ANI_FLAG%02d_NOTGET", m_nNumber);
	if (kingdom::cKingdomModel* m = pModel(1)) m->RegistAnm(0u, key);
	Disable();                  // vtable +0x54
	if (m_nNumber == 1) Enable();   // vtable +0x58 (only FUN_80072b20 calls it)
}

// DOL 0x80076704 / 0x80076640 / 0x8007657c / 0x800764b8 / 0x800763f4 / 0x80076330 / 0x8007626c / 0x800761a8 / 0x800760e4 / 0x80076020
// DOL 0x80075f5c / 0x80075e98 / 0x80075dd4 / 0x80075d10 / 0x80075c4c / 0x80075b88 / 0x80075ac4 / 0x80075a00 / 0x8007593c / 0x80075878
// (numbers 01..20)
const char* cTaskItemFlagNumber::GetName() const
{
	static char name[20][16];
	if (m_bTaken) return "Flag";
	char* s = name[m_nNumber - 1];
	std::snprintf(s, 16, "NumberFlag%02d", m_nNumber);
	return s;
}

// DOL 0x80072c60
void cTaskItemFlagNumber::Enable()
{
	if (kingdom::cKingdomModel* m = pModel(1)) m->ReleaseAnmTexPat(0u);   // FUN_8001b1bc(model 1, 0)
	cTaskItemFlag::Enable();   // FUN_800b31e0
}

}
