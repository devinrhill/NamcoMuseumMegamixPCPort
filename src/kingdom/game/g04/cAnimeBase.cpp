#include "kingdom/game/g04/cAnimeBase.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "kingdom/cKingdomModel.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g04
{

namespace
{
std::string Key(const kingdom::cKingdomModel* m, uint slot, int kind)
{
	char b[64];
	std::snprintf(b, sizeof b, "%p|%u|%d", (const void*)m, slot, kind);
	return b;
}
}

int cAnimeBase::Kind(const char* name)
{
	static std::map<std::string, int> cache;
	auto it = cache.find(name);
	if (it != cache.end()) return it->second;
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	int kind = -1;
	static const char* const MAGIC[5] = {"CHR0", "VIS0", "CLR0", "PAT0", "SRT0"};
	if (d != nullptr)
		for (int k = 0; k < 5 && kind < 0; k++)
			if (std::search(d, d + size, MAGIC[k], MAGIC[k] + 4) != d + size) kind = k;
	cache[name] = kind;
	return kind;
}

bool cAnimeBase::IsAttached(kingdom::cKingdomModel* m, const char* name, uint slot) const
{
	auto it = m_attached.find(Key(m, slot, Kind(name)));
	return it != m_attached.end() && it->second == name;
}

void cAnimeBase::Regist(kingdom::cKingdomModel* m, const char* name, uint slot)
{
	if (m == nullptr) return;
	m->RegistAnm(slot, name);
	m_attached[Key(m, slot, Kind(name))] = name;
}

void cAnimeBase::Attach(kingdom::cKingdomModel* m, const char* name, uint slot)
{
	if (!IsAttached(m, name, slot)) Regist(m, name, slot);
}

void cAnimeBase::Blend(float rate, kingdom::cKingdomModel* m)
{
	if (m == nullptr) return;
	float w0 = m->GetWeightAnmChr(0u), w1 = m->GetWeightAnmChr(1u);
	w1 += rate;
	if (w1 >= 1.0f) w1 = 1.0f;   // FLOAT_8047ab48
	w0 -= rate;
	if (w0 <= 0.0f) w0 = 0.0f;   // FLOAT_8047ab4c
	m->SetWeightAnmChr(0u, w0);
	m->SetWeightAnmChr(1u, w1);
}

void cAnimeBase::SetWeight(float w, kingdom::cKingdomModel* m, uint slot)
{
	if (m != nullptr) m->SetWeightAnmChr(slot, w);
}

float cAnimeBase::GetWeight(kingdom::cKingdomModel* m, uint slot)
{
	return m != nullptr ? m->GetWeightAnmChr(slot) : 0.0f;
}

bool cAnimeBase::ChrEnd(kingdom::cKingdomModel* m, uint slot)
{
	return m != nullptr && m->GetFrameMaxAnmChr(slot) <= m->GetFrameAnmChr(slot);
}

bool cAnimeBase::ClrEnd(kingdom::cKingdomModel* m, uint slot)
{
	return m != nullptr && m->GetFrameMaxMatClr(slot) <= m->GetFrameMatClr(slot);
}

}
