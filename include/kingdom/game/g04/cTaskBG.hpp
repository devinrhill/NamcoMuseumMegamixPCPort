#pragma once

#include "kingdom/cTaskModel.hpp"

// Gator Panic Remix (g04) the scenery: kingdom::game::g04::cTaskBG (0x1d0 bytes, vtable 0x80449628, a cTaskModel; built by the app's 0x800931c0,
// priority 0x10005). Init (vtable +0x3c, 0x8019ce0c):
//   model 0 = BIN_BG_01 (the placement skeleton, hidden: 0x8001a330), light set 0, fog 0
//   model 1 = BIN_BG_NEAR at model 0's node near_bg_set (0x8001dd54: the bind translate), clip BIN_BG_NEAR_CHA in slot 0
//   model 2 = BIN_BG_ISLAND at node island_set
//   model 3 = BIN_BG_MDL at node bg_set, clips BIN_BG_ANIME and BIN_BG_TSA in slot 0
// Term (+0x40, 0x8019d028) does nothing; Execute / Draw are cTaskModel's (the clips advance: natively the model task's frame update).
namespace kingdom::game::g04
{
class cTaskBG : public kingdom::cTaskModel
{
public:
	cTaskBG();
	void Init();                                  // DOL 0x8019ce0c
	bool Execute(cJetTask* pParam) override;
	// the placement nodes of model 0 (FUN_8001dd54: bind translations)
	Vector4 WaniNode(uint place);                 // DOL 0x8019d02c: wani_A..D (a gator's hole)
	Vector4 MizuNode();                           // DOL 0x8019d0c8: "mizu" of model 3 (the water line)
	Vector4 HideNode(uint place);                 // DOL 0x8019d10c: hide_A..D
	Vector4 ShowNode(uint place);                 // DOL 0x8019d180: show_A..D
	Vector4 BirdNode();                           // DOL 0x8019d1f4: bird_set
	Vector4 RenseNode();                          // DOL 0x8019d238: rense_set (the lens flare)
	Vector4 KameNode();                           // the tortoise builder 0x80093260: kame_set
};
}
