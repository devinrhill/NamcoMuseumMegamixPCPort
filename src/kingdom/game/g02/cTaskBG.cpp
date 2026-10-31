#include "kingdom/game/g02/cTaskBG.hpp"

#include <cstring>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g02
{

namespace
{
bool Exists(const char* name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}

// a "set" skeleton (FUN_8001e790): the bind-pose position of a node, by name (FUN_8001dd54) or by index (FUN_8001de14)
struct cSet
{
	wii::Brres brres;
	wii::Mdl0 skel;
	bool ok{};
	explicit cSet(const char* name)
	{
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(name, size);
		ok = d != nullptr && brres.Load(d, size) && !brres.ModelNames().empty() && skel.Load(brres, brres.ModelNames()[0]);
	}
	Vector4 Pos(const wii::Mdl0Node& n) const { return Vector4(n.model[3], n.model[7], n.model[11], 1.0f); }   // UNVERIFIED: translation only
	Vector4 ByName(const char* node) const
	{
		if (ok)
			for (const wii::Mdl0Node& n : skel.nodes)
				if (n.name == node) return Pos(n);
		return Vector4(0.0f, 0.0f, 0.0f, 1.0f);   // FLOAT_8047a8cc / 8047a8d0
	}
	Vector4 ByIndex(uint i) const { return ok && i < skel.nodes.size() ? Pos(skel.nodes[i]) : Vector4(0.0f, 0.0f, 0.0f, 1.0f); }
};
}

// DOL 0x8005f8e8 [audited] (priority 0x10001)
cTaskBG::cTaskBG()
{
	SetName("g02::cTaskBG");
	nDrawPriority(0x10001u);
	uint idx = 0;
	auto add = [&](const char* model, const char* anim, const Vector4& pos) {
		if (!Exists(model)) return;
		cKingdomModel* m = CreateModel(idx++);
		m->RegistModel(model, 0x36f);
		m->SetLightSetIdx(0);
		m->SetFogIdx(0);
		if (anim != nullptr && Exists(anim)) m->RegistAnm(0u, anim);   // FUN_8000bca0 = the entry exists
		m->vTrans() = pos;
	};
	static const char* const MODEL[12] = {"STAGE_BG_1A", "STAGE_BG_1B", "STAGE_BG_1C", "STAGE_BG_1D", "STAGE_BG_2A", "STAGE_BG_2B",
	                                      "STAGE_BG_2C", "STAGE_BG_2D", "STAGE_BG_3A", "STAGE_BG_3B", "STAGE_BG_3C", "STAGE_BG_3D"};
	static const char* const NODE[12] = {"_1a", "_1b", "_1c", "_1d", "_2a", "_2b", "_2c", "_2d", "_3a", "_3b", "_3c", "_3d"};
	// 0x8036b5f0: the clips (all 12 CRCs are these names, each attached only when the stage archive has it)
	static const char* const ANIM[12] = {"STAGE_BG_1A_ANIM", "STAGE_BG_1B_ANIM", "STAGE_BG_1C_ANIM", "STAGE_BG_1D_ANIM", "STAGE_BG_2A_ANIM", "STAGE_BG_2B_ANIM",
	                                     "STAGE_BG_2C_ANIM", "STAGE_BG_2D_ANIM", "STAGE_BG_3A_ANIM", "STAGE_BG_3B_ANIM", "STAGE_BG_3C_ANIM",
	                                     "STAGE_BG_3D_ANIM"};
	{
		cSet set("STAGE_BG_SET");
		for (int i = 0; i < 12; i++) add(MODEL[i], ANIM[i], set.ByName(NODE[i]));
	}
	add("STAGE_SKY", "STAGE_SKY_ANIM", cSet("STAGE_SKY_SET").ByIndex(1));   // +0x1dc
	{
		static const char* const OBJ[8] = {"STAGE_BG_OUT_OBJ_1", "STAGE_BG_OUT_OBJ_2", "STAGE_BG_OUT_OBJ_3", "STAGE_BG_OUT_OBJ_4",
		                                   "STAGE_BG_OUT_OBJ_5", "STAGE_BG_OUT_OBJ_6", "STAGE_BG_OUT_OBJ_7", "STAGE_BG_OUT_OBJ_8"};
		static const char* const OBJ_ANIM[8] = {"STAGE_BG_OUT_OBJ_1_ANIM", "STAGE_BG_OUT_OBJ_2_ANIM", "STAGE_BG_OUT_OBJ_3_ANIM",
		                                        "STAGE_BG_OUT_OBJ_4_ANIM", "STAGE_BG_OUT_OBJ_5_ANIM", "STAGE_BG_OUT_OBJ_6_ANIM",
		                                        "STAGE_BG_OUT_OBJ_7_ANIM", "STAGE_BG_OUT_OBJ_8_ANIM"};
		static const char* const OB[8] = {"ob1", "ob2", "ob3", "ob4", "ob5", "ob6", "ob7", "ob8"};
		cSet set("STAGE_BG_OUT_OBJ_SET");
		for (int i = 0; i < 8; i++) add(OBJ[i], OBJ_ANIM[i], set.ByName(OB[i]));
	}
	add("STAGE_BG_OUT", nullptr, cSet("STAGE_BG_OUT_SET").ByIndex(1));
}

// DOL 0x8006002c [audited]: every handle detaches its particle (vtable +0x10 with 1)
void cTaskBG::Term()
{
	for (auto& h : m_Ptk)
		if (h != nullptr) h->Detach(true);
}

// DOL 0x8006016c [audited]
bool cTaskBG::ResetPtk()
{
	if (!Exists("STAGE_BG_PTK_POINT")) return false;   // FUN_8000bca0(0xefad9461)
	cSet set("STAGE_BG_PTK_POINT");                     // FUN_8001e790(.., 0x36f)
	if (!set.ok) return false;
	int root = -1;
	for (const wii::Mdl0Node& n : set.skel.nodes)
		if (n.name == "ef_butterfly01") { root = n.id; break; }   // ResMdl::GetResNode
	if (root < 0) return false;
	// the children through ResNode +0x60 (first child) / +0x64 (next sibling): in the node list order
	uint i = 0;
	for (const wii::Mdl0Node& n : set.skel.nodes)
	{
		if (n.parent != root) continue;
		if (m_Ptk.size() < i + 1) m_Ptk.resize(i + 1);
		if (m_Ptk[i] == nullptr) m_Ptk[i] = std::make_unique<kingdom::cKingdomPtkHnd>();
		// FUN_80021a9c(delay 0 (FLOAT_8047a8cc), hnd, slot 0x28, onetime 0, depend 0, clipping 1, the node's translation (+0x38), no rotate / scale)
		m_Ptk[i]->Generate(0x28u, false, false, true, 0.0f, Vector4(n.translate[0], n.translate[1], n.translate[2], 1.0f));
		i++;
	}
	return i != 0;   // the DOL returns 1 only when the node has children
}

// DOL 0x800600a4 [audited]: every model's clips step, unless the app is paused (+0x1b4), the stage end runs (+0x284) or HOME is held
bool cTaskBG::Execute(cJetTask* pParam)
{
	const cRallyUnitHost* host = KingdomGlobal::KingSys != nullptr ? dynamic_cast<const cRallyUnitHost*>(KingdomGlobal::KingSys->pAppMain()) : nullptr;
	const bool home = (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
	if (!home && (host == nullptr || (!host->IsUnitPause() && !host->IsUnitStageEnd()))) UpdateFrame();
	return cTaskModel::Execute(pParam);
}

}
