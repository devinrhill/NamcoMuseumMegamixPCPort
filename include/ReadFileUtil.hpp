#pragma once

#include <cstdint>
#include <cstring>

#include "common.hpp"
#include "kingdom/game/g03/sEnemyAreaData.hpp"
#include "kingdom/game/g03/sEnemyRouteData.hpp"
#include "kingdom/game/g03/sSaveDataBox.hpp"
#include "kingdom/game/g03/sSaveDataFallArea.hpp"
#include "kingdom/game/g03/sSaveDataWater.hpp"
#include "kingdom/game/g03/sSaveDataCameraArea.hpp"
#include "kingdom/game/g03/sSaveDataGate.hpp"
#include "kingdom/game/g03/sSaveDataGoal.hpp"
#include "kingdom/game/g03/sSaveDataItem.hpp"
#include "kingdom/game/g03/sSaveDataPoint.hpp"
#include "kingdom/game/g03/sSaveDataStart.hpp"
#include "kingdom/game/g03/sSaveDataWind.hpp"

// Replacement for the Unity ReadFileUtil (Marshal.PtrToStructure over the stage data files). The stage "*_SET_DAT" files are arrays of fixed-size
// records of big-endian 32-bit words (floats / ints); the file is padded to a multiple of 32 bytes, so the count is size / record size.
// A record type is described by `Rec<T>::Size` (bytes in the file, = the sum of the C# fields, bool = 4 bytes) and `Rec<T>::Read`.
namespace readfile
{
struct Words
{
	const std::uint8_t* p;
	std::uint32_t U() { std::uint32_t v = ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; p += 4; return v; }
	int I() { return (int)U(); }
	float F() { std::uint32_t v = U(); float f; std::memcpy(&f, &v, 4); return f; }
	Vector4 V4() { float x = F(), y = F(), z = F(), w = F(); return Vector4(x, y, z, w); }
};
template<class T> struct Rec;   // specialised below
}

using kingdom::game::g03::sEnemyAreaData;
using kingdom::game::g03::sEnemyRouteData;
using kingdom::game::g03::sSaveDataBox;
using kingdom::game::g03::sSaveDataFallArea;
using kingdom::game::g03::sSaveDataWater;
using kingdom::game::g03::sSaveDataCameraArea;
using kingdom::game::g03::sSaveDataGate;
using kingdom::game::g03::sSaveDataGoal;
using kingdom::game::g03::sSaveDataItem;
using kingdom::game::g03::sSaveDataPoint;
using kingdom::game::g03::sSaveDataStart;
using kingdom::game::g03::sSaveDataWind;

namespace readfile
{
template<> struct Rec<sSaveDataItem> { static constexpr int Size = 48;
	static void Read(Words w, sSaveDataItem& t) { t.kind = w.U(); t.flag = w.U(); t.empty[0] = w.U(); t.empty[1] = w.U(); t.trans = w.V4(); for (auto& d : t.dummy) d = w.U(); } };
template<> struct Rec<sSaveDataGoal> { static constexpr int Size = 32;
	static void Read(Words w, sSaveDataGoal& t) { t.trans = w.V4(); t.rotY = w.I(); for (auto& d : t.dummy) d = w.U(); } };
template<> struct Rec<sSaveDataStart> { static constexpr int Size = 48;
	static void Read(Words w, sSaveDataStart& t) { for (auto& d : t.empty) d = w.U(); t.isDrop = w.U() != 0; t.trans = w.V4(); t.rotY = w.I(); for (auto& d : t.dummy) d = w.U(); } };
template<> struct Rec<sSaveDataGate> { static constexpr int Size = 48;
	static void Read(Words w, sSaveDataGate& t) { t.cookie = w.U(); for (auto& d : t.empty) d = w.U(); t.trans = w.V4(); t.rotY = w.I(); t.kind = w.U(); t.no = w.U(); t.dummy[0] = w.U(); } };
template<> struct Rec<sSaveDataBox> { static constexpr int Size = 48;
	static void Read(Words w, sSaveDataBox& t) { t.kind = w.U(); t.treasure = w.U(); t.empty[0] = w.U(); t.empty[1] = w.U(); t.trans = w.V4(); t.rotY = w.I(); for (auto& d : t.dummy) d = w.U(); } };
template<> struct Rec<sSaveDataPoint> { static constexpr int Size = 32;
	static void Read(Words w, sSaveDataPoint& t) { t.trans = w.V4(); t.rotY = w.I(); for (auto& d : t.dummy) d = w.U(); } };
template<> struct Rec<sSaveDataCameraArea> { static constexpr int Size = 84;
	static void Read(Words w, sSaveDataCameraArea& t) { for (auto& d : t.vnTrans) d = w.I(); for (auto& d : t.vnRotate) d = w.I(); for (auto& d : t.vnScale) d = w.I(); t.nArea = w.I(); for (auto& d : t.dummy) d = w.I(); } };
template<> struct Rec<sSaveDataWind> { static constexpr int Size = 64;
	static void Read(Words w, sSaveDataWind& t) { for (auto& d : t.vnTrans) d = w.I(); for (auto& d : t.vnRotate) d = w.I(); for (auto& d : t.vnScale) d = w.I(); for (auto& d : t.vnRotWind) d = w.I(); t.nForce = w.I(); for (auto& d : t._pad) d = w.U(); } };
template<> struct Rec<sSaveDataFallArea> { static constexpr int Size = 64;
	static void Read(Words w, sSaveDataFallArea& t) { for (auto& d : t.vnTrans) d = w.I(); for (auto& d : t.vnRotate) d = w.I(); for (auto& d : t.vnScale) d = w.I(); t.nAreaCheckKind = w.U(); for (auto& d : t._pad) d = w.U(); } };
template<> struct Rec<sSaveDataWater> { static constexpr int Size = 64;
	static void Read(Words w, sSaveDataWater& t) { for (auto& d : t.vnTrans) d = w.I(); for (auto& d : t.vnRotate) d = w.I(); for (auto& d : t.vnScale) d = w.I(); for (auto& d : t.vnRotWater) d = w.I(); t.nForce = w.I(); for (auto& d : t._pad) d = w.U(); } };
// enemy area file (BIN_GHOST_AREA_*_SET_DAT, BIN_BOSS_AREA_SET_DAT: 5152 bytes): header {nAreaNum, nPointNum, nEnmeyNum, pad}, 48 areas (kind, 3 pad, 4 positions),
// 64 move points (Vector4), 8 enemies (pos, type, option, ride, pad)
template<> struct Rec<sEnemyAreaData> { static constexpr int Size = 5152;
	static void Read(Words w, sEnemyAreaData& t)
	{
		t.nAreaNum = w.I(); t.nPointNum = w.I(); t.nEnmeyNum = w.I(); t.empty[0] = w.U();
		for (auto& a : t.Area) { a.kind = w.U(); for (auto& d : a.empty) d = w.U(); for (auto& p : a.pos) p = w.V4(); }
		for (auto& p : t.Point) p.point = w.V4();
		for (auto& e : t.Enemy) { e.pos = w.V4(); e.type = w.I(); e.option = w.I(); e.ride = w.I(); e.empty[0] = w.U(); }
	} };
// enemy route file (BIN_GHOST_ROUTE_*_SET_DAT: 1088 bytes): header {nRouteNum, nBombStatus, 2 pad}, 32 route points (speed, 3 pad, pos), 1 enemy
template<> struct Rec<sEnemyRouteData> { static constexpr int Size = 1088;
	static void Read(Words w, sEnemyRouteData& t)
	{
		t.nRouteNum = w.I(); t.nBombStatus = w.I(); for (auto& d : t.empty) d = w.U();
		for (auto& p : t.sEnemyRoute) { p.speed = w.I(); for (auto& d : p.empty) d = w.U(); p.pos = w.V4(); }
		t.sEnemy.pos = w.V4(); t.sEnemy.type = w.I(); t.sEnemy.option = w.I(); t.sEnemy.ride = w.I(); t.sEnemy.empty[0] = w.U();
	} };
}

class ReadFileUtil
{
public:
	// (bytes, first record, size of the first record) - the single record files (start, goal)
	template<class T> static bool Convert(const std::vector<byte>& bytes, T*& out, int& size)
	{
		out = nullptr;
		size = 0;
		if ((int)bytes.size() < readfile::Rec<T>::Size) return false;
		out = new T();
		readfile::Rec<T>::Read({bytes.data()}, *out);
		size = readfile::Rec<T>::Size;
		return true;
	}
	// (bytes, records, count, size of one record)
	template<class T> static bool Convert(const std::vector<byte>& bytes, std::vector<T*>& outs, int& num, int& size)
	{
		outs.clear();
		size = readfile::Rec<T>::Size;
		num = (int)bytes.size() / size;
		for (int i = 0; i < num; i++)
		{
			T* t = new T();
			readfile::Rec<T>::Read({bytes.data() + (std::size_t)i * size}, *t);
			outs.push_back(t);
		}
		return true;
	}
};
