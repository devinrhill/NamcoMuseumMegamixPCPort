#include "kingdom/game/g04/cTaskBG.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/g04Util.hpp"

namespace kingdom::game::g04
{

namespace
{
// FUN_8001dd54: the bind translate of a node of the placement skeleton (model 0)
Vector4 NodePos(cKingdomModel* set, const char* node)
{
	return ResNodeTrans(set, node);
}
}

Vector4 cTaskBG::WaniNode(uint place)
{
	static const char* const N[4] = {"wani_A", "wani_B", "wani_C", "wani_D"};   // 0x8037c3d8
	return NodePos(pModel(0u), N[place & 3]);
}

Vector4 cTaskBG::MizuNode()
{
	return NodePos(pModel(3u), "mizu");   // DAT_8037c442
}

Vector4 cTaskBG::HideNode(uint place)
{
	static const char* const N[4] = {"hide_A", "hide_B", "hide_C", "hide_D"};   // 0x8037c3e8
	return NodePos(pModel(0u), N[place & 3]);
}

Vector4 cTaskBG::ShowNode(uint place)
{
	static const char* const N[4] = {"show_A", "show_B", "show_C", "show_D"};   // 0x8037c3f8
	return NodePos(pModel(0u), N[place & 3]);
}

Vector4 cTaskBG::BirdNode()
{
	return NodePos(pModel(0u), "bird_set");
}

Vector4 cTaskBG::RenseNode()
{
	return NodePos(pModel(0u), "rense_set");
}

Vector4 cTaskBG::KameNode()
{
	return NodePos(pModel(0u), "kame_set");   // 0x8036d4fc
}

// the app's builder 0x800931c0 (the vtable +0x3c call = Init; priority 0x10005 set by the builder)
cTaskBG::cTaskBG()
{
	SetName("g04::cTaskBG");
	Init();
}

// DOL 0x8019ce0c. 0x8001e16c (register the model with the app's scene root) is implicit natively.
void cTaskBG::Init()
{
	cKingdomModel* set = CreateModel(0u);   // FUN_8009991c(+0x88, 0)
	set->RegistModel("BIN_BG_01", 0x36f);   // 0x04db6606
	set->Invisible();                       // FUN_8001a330: only the placement skeleton
	set->SetLightSetIdx(0);
	set->SetFogIdx(0);

	cKingdomModel* m = CreateModel(1u);
	m->RegistModel("BIN_BG_NEAR", 0x36f);   // 0x074d2c70
	m->vTrans() = NodePos(set, "near_bg_set");   // s_near_bg_set_8037c424
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);
	m->RegistAnm(0u, "BIN_BG_NEAR_CHA");    // 0x7856fc89, FUN_8001a4d8(m, 0, crc, 0, 1)

	m = CreateModel(2u);
	m->RegistModel("BIN_BG_ISLAND", 0x36f);   // 0x5dda70cb
	m->vTrans() = NodePos(set, "island_set");
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);

	m = CreateModel(3u);
	m->RegistModel("BIN_BG_MDL", 0x36f);   // 0x4a6eead5
	m->vTrans() = NodePos(set, "bg_set");
	m->SetLightSetIdx(0);
	m->RegistAnm(0u, "BIN_BG_ANIME");      // 0x12ae2974
	m->SetFogIdx(0);
	m->RegistAnm(0u, "BIN_BG_TSA");        // 0x22ab8a01
}

// cTaskModel::Execute 0x8009a4ec (FUN_8001e5c8 per model: the clips advance)
bool cTaskBG::Execute(cJetTask* pParam)
{
	UpdateFrame();
	return cTaskModel::Execute(pParam);
}

}
