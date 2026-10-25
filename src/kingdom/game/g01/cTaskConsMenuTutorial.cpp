#include "kingdom/game/g01/cTaskConsMenuTutorial.hpp"

#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::g01
{

namespace
{
using kingdom::game::menu::sTutorialData;
using kingdom::game::menu::sTutorialPage;

// Archive entries of g01menu.lzs (the DOL passes their CRC32s; reversing/dol/g01_archives.txt)
const char* const ANT_COMMON_TUTORIAL = "ANT_COMMON_TUTORIAL";   // 0x097a4e77 (ctor argument)
const char* const ANT_TUTORIAL = "ANT_TUTORIAL";                 // 0x2c431b40 (every page)
const char* const TEXT_TUTORIAL = "TEXT_TUTORIAL";               // 0xd494f3bd (page text CSV)

// sTutorialPage tables (12 bytes each: anim no, text row, layout CRC). TEXT_TUTORIAL rows 1..3 = single player, 5..7 = "Cooperate ..." (multi).
sTutorialPage s_PageSingle[3] = {   // DOL 0x80437eb8
	{-1, 1, ANT_TUTORIAL},
	{0, 2, ANT_TUTORIAL},
	{1, 3, ANT_TUTORIAL},
};
sTutorialPage s_PageMulti[3] = {    // DOL 0x80437edc
	{-1, 5, ANT_TUTORIAL},
	{0, 6, ANT_TUTORIAL},
	{1, 7, ANT_TUTORIAL},
};

// sTutorialData (page count, text CSV CRC, page table)
sTutorialData* DataSingle()   // DOL 0x803705c8
{
	static sTutorialData s{3u, TEXT_TUTORIAL, {&s_PageSingle[0], &s_PageSingle[1], &s_PageSingle[2]}};
	return &s;
}
sTutorialData* DataMulti()    // DOL 0x803705d4
{
	static sTutorialData s{3u, TEXT_TUTORIAL, {&s_PageMulti[0], &s_PageMulti[1], &s_PageMulti[2]}};
	return &s;
}
}

// DOL 0x800bfb9c: base ctor 0x80145890, then the virtual RegistData (slot +0x54 = 0x80145cbc) with (pCursor, ANT_COMMON_TUTORIAL, data):
// bMulti == 0 -> 0x803705c8, else 0x803705d4. The port's RegistData takes the cursor separately (SetCursor = the DOL's store to +0x1e0).
cTaskConsMenuTutorial::cTaskConsMenuTutorial(bool bMulti, kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	if (!bMulti)
	{
		RegistData(ANT_COMMON_TUTORIAL, DataSingle());
	}
	else
	{
		RegistData(ANT_COMMON_TUTORIAL, DataMulti());
	}
}

// DOL 0x800bfc54: only the inlined base member destructors (+0x2c8 csv, +0x214 font writer)
cTaskConsMenuTutorial::~cTaskConsMenuTutorial()
{
}

}
