// gotch::CDataStorage (RSO2 0x80d02424 SaveRequest / 0x80d02634 FrameUpdate): a save goes to the bridge after one FrameUpdate, a load of an
// empty / corrupt bridge zero-fills the blocks, a load of a valid one comes back byte for byte.
#include <cstdio>
#include <cstring>

#include "gotch/DataStorage.hpp"

using gotch::CDataStorage;

static int g_fail = 0;
static void Check(bool ok, const char* what)
{
	if (!ok) { std::printf("FAIL: %s\n", what); g_fail++; }
}

int main()
{
	std::memset(gotch::BridgeSave(), 0, gotch::kBridgeSaveSize);
	{
		// first boot: the bridge is zero -> state 3, blocks zero
		CDataStorage s;
		s.magicOffset = 0x28;
		s.sumOffset = 0x2c;
		s.LoadRequest();
		Check(gotch::GetDataStorageBusy(), "busy while loading");
		Check(s.FrameUpdate() == 3, "empty bridge = load failed (3)");
		s.ActivateLoadedData();
		Check(s.Data()[0x30] == 0, "zero filled");
		// a save: stamped, then copied out on the next frame
		s.Data()[0x30] = 0x12;
		s.Data()[0x100] = 0x34;
		Check(s.SaveRequest() == 2, "save pending (2)");
		Check(gotch::BridgeSave()[0x30] == 0, "not copied before the frame");
		Check(s.FrameUpdate() == 0, "save done (0)");
		const std::uint8_t* b = gotch::BridgeSave();
		Check(b[0x28] == 0 && b[0x29] == 0 && b[0x2a] == 0 && b[0x2b] == 0x1f, "magic 0x1f big-endian at +0x28");
		Check(b[0x2f] == 0x46 && b[0x2e] == 0, "sum 0x12 + 0x34 big-endian at +0x2c");
		Check(b[0x30] == 0x12 && b[0x100] == 0x34, "data in the bridge");
		Check(s.SaveRequest() == 0, "unchanged data: no save");
	}
	{
		// the next session reads it back
		CDataStorage s;
		s.magicOffset = 0x28;
		s.sumOffset = 0x2c;
		s.LoadRequest();
		Check(s.FrameUpdate() == 0, "valid bridge = load ok (0)");
		s.ActivateLoadedData();
		Check(s.Data()[0x30] == 0x12 && s.Data()[0x100] == 0x34, "data back");
	}
	{
		// a corrupt sum
		gotch::BridgeSave()[0x200] = 1;
		CDataStorage s;
		s.magicOffset = 0x28;
		s.sumOffset = 0x2c;
		s.LoadRequest();
		Check(s.FrameUpdate() == 3, "bad sum = load failed (3)");
	}
	if (g_fail == 0) std::printf("gotch storage tests passed\n");
	return g_fail != 0;
}
