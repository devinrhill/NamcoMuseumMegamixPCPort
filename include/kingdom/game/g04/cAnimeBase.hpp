#pragma once

#include <map>
#include <string>

#include "common.hpp"

namespace kingdom { class cKingdomModel; }

// Gator Panic Remix (g04) clip helper: kingdom::game::g04::cAnimeBase (vtable 0x80394090), a sub-object (+0x1cc) of cTaskBG, cTaskEnemy,
// cTaskPacmanBase and cTaskTortoise; the game calls it through the secondary vtable (+0xc .. +0x24):
//   +0x0c IsAttached 0x80090650  the resource of the slot (of the clip's kind: CHR0 / VIS0 / CLR0 / PAT0 / SRT0) is that archive entry
//   +0x10 Attach     0x80090cf8  RegistAnm (0x8001a544) unless IsAttached
//   +0x14 Blend      0x80090d94  cross fade: weight of CHR slot 1 += rate (max 1, 0x80199bd8), slot 0 -= rate (min 0, 0x80199c04)
//   +0x18 SetWeight  0x80090e74  = SetWeightAnmChr (0x8001d994);  +0x1c GetWeight 0x80090e80 = GetWeightAnmChr (0x8001da38)
//   +0x20 ChrEnd     0x80090e8c  GetFrameAnmChr (0x8001c2e0) >= GetFrameMaxAnmChr (0x8001c630)
//   +0x24 ClrEnd     0x80090ef4  GetFrameMatClr (0x8001c42c) >= GetFrameMaxMatClr (0x8001c804)
// Natively IsAttached remembers the entry last attached to (model, slot, kind); the kind comes from the entry's sub-file magic.
namespace kingdom::game::g04
{
class cAnimeBase
{
public:
	bool IsAttached(kingdom::cKingdomModel* m, const char* name, uint slot) const;   // DOL 0x80090650
	void Attach(kingdom::cKingdomModel* m, const char* name, uint slot);           // DOL 0x80090cf8
	void Blend(float rate, kingdom::cKingdomModel* m);                             // DOL 0x80090d94
	void SetWeight(float w, kingdom::cKingdomModel* m, uint slot);                 // DOL 0x80090e74
	float GetWeight(kingdom::cKingdomModel* m, uint slot);                         // DOL 0x80090e80
	bool ChrEnd(kingdom::cKingdomModel* m, uint slot);                             // DOL 0x80090e8c
	bool ClrEnd(kingdom::cKingdomModel* m, uint slot);                             // DOL 0x80090ef4
	// RegistAnm (0x8001a4d8) that keeps the bookkeeping of IsAttached in step
	void Regist(kingdom::cKingdomModel* m, const char* name, uint slot);
	static int Kind(const char* name);   // 0 CHR0, 1 VIS0, 2 CLR0, 3 PAT0, 4 SRT0, -1 unknown / missing

private:
	std::map<std::string, std::string> m_attached;   // "model|slot|kind" -> entry name
};
}
