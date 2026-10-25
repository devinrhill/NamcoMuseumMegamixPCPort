#include "jet/JetUtil.hpp"

#include <cstring>

#include "CSharpCompat.hpp"
#include "jet/uDatHead.hpp"

namespace jet
{

uDatTex* JetUtil::anpTexPtr(void* ptr, int n)
{
	return (uDatTex*)((byte*)ptr + ((uDatHead*)ptr)->head.tex_offset + sizeof(uDatTex) * n);
}

int JetUtil::anpTexCount(void* ptr)
{
	return ((uDatHead*)ptr)->head.tex_max;
}

string JetUtil::anpTexTexName(void* ptr, int n)
{
	uDatTex* ptr2 = anpTexPtr(ptr, n);
	byte array[16] = {};
	for (int i = 0; i < 16; i++)
	{
		array[i] = ptr2->tex.img_name[i];
	}
	return CSharpCompat::Trim(string((const char*)array, 16), '\0');
}

uDatPat* JetUtil::anpPatPtr(void* ptr, int n)
{
	return (uDatPat*)((byte*)ptr + ((uDatHead*)ptr)->head.pat_offset + sizeof(uDatPat) * n);
}

int JetUtil::anpPatCount(void* ptr)
{
	return ((uDatHead*)ptr)->head.pat_max;
}

int JetUtil::anpPatLayCount(void* ptr, int n)
{
	uDatPat* ptr2 = anpPatPtr(ptr, n);
	return ptr2->pat.lay_index_end - ptr2->pat.lay_index_sta;
}

uDatLayP* JetUtil::anpLayPtr(void* ptr, int n)
{
	return (uDatLayP*)((byte*)ptr + ((uDatHead*)ptr)->head.lay_offset + sizeof(uDatLayP) * n);
}

uDatLayP* JetUtil::anpPatLayPtr(void* ptr, int p, int l)
{
	uDatPat* ptr2 = anpPatPtr(ptr, p);
	return anpLayPtr(ptr, ptr2->pat.lay_index_sta + l);
}

uDatAnmP* JetUtil::anpAnmPtr(void* ptr, int n)
{
	return (uDatAnmP*)((byte*)ptr + ((uDatHead*)ptr)->head.anm_offset + sizeof(uDatAnmP) * n);
}

int JetUtil::anpAnmCount(void* ptr)
{
	return ((uDatHead*)ptr)->head.anm_max;
}

int JetUtil::anpAnmSeqCount(void* ptr, int n)
{
	uDatAnmP* ptr2 = anpAnmPtr(ptr, n);
	return ptr2->anm.index_end - ptr2->anm.index_sta;
}

uDatSeqP* JetUtil::anpAnmSeqPtr(void* ptr, int n, int s)
{
	uDatAnmP* ptr2 = anpAnmPtr(ptr, n);
	return (uDatSeqP*)((byte*)ptr + ((uDatHead*)ptr)->head.anm_offset + sizeof(uDatAnmP) * anpAnmCount(ptr) + sizeof(uDatSeqP) * (ptr2->anm.index_sta + s));
}

uDatSeqP* JetUtil::anpAnmSeqBSPtr(void* ptr, int n, float f)
{
	uDatAnmP* ptr2 = anpAnmPtr(ptr, n);
	uDatSeqP* ptr3 = (uDatSeqP*)((byte*)ptr + ((uDatHead*)ptr)->head.anm_offset + sizeof(uDatAnmP) * anpAnmCount(ptr) + sizeof(uDatSeqP) * ptr2->anm.index_sta);
	int num = 0;
	int num2 = ptr2->anm.index_end - ptr2->anm.index_sta + 1;
	while (num2 - num > 1)
	{
		int num3 = (num2 + num) / 2;
		if (f < (float)ptr3[num3].seq.frm)
		{
			num2 = num3;
		}
		else
		{
			num = num3;
		}
	}
	return ptr3 + num;
}

static uint32_t Swap32(uint32_t v)
{
	return (v >> 24) | ((v >> 8) & 0xff00u) | ((v << 8) & 0xff0000u) | (v << 24);
}

// Swaps the 4-byte word at `p` in place.
static void SwapWord(byte* p)
{
	uint32_t v;
	std::memcpy(&v, p, 4);
	v = Swap32(v);
	std::memcpy(p, &v, 4);
}

static uint32_t ReadBE(const byte* p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

bool JetUtil::anpToHostEndian(std::vector<byte>& data)
{
	constexpr int HEAD = 48;
	if (data.size() < (size_t)HEAD || std::memcmp(data.data(), "ANP ", 4) != 0)
	{
		return false;
	}
	byte* base = data.data();
	// Fixed layout: tex_offset is always 48, right after the header. Read as big endian it must say so.
	if (ReadBE(base + 8 + 4 * 5) != (uint32_t)HEAD)
	{
		return true; // already host order
	}
	for (int i = 0; i < 10; i++) SwapWord(base + 8 + 4 * i); // tex/pat/lay/anm/seq max + offsets

	const uint32_t patMax = *(int*)(base + 12), layMax = *(int*)(base + 16), anmMax = *(int*)(base + 20), seqMax = *(int*)(base + 24);
	// header: ver[8], tex/pat/lay/anm/seq _max at +8..+24, tex/pat/lay/anm/seq _offset at +28..+44
	const uint32_t patOff = *(int*)(base + 32), layOff = *(int*)(base + 36), anmOff = *(int*)(base + 40);
	auto inside = [&](uint64_t off, uint64_t n, uint64_t size) { return off + n * size <= data.size(); };
	if (!inside(patOff, patMax, 16) || !inside(layOff, layMax, 96) || !inside(anmOff, anmMax, 32))
	{
		return false;
	}
	for (uint32_t i = 0; i < patMax; i++)  // lay_max, lay_index_sta, lay_index_end (+4 unused)
		for (int k = 0; k < 3; k++) SwapWord(base + patOff + 16 * i + 4 * k);
	for (uint32_t i = 0; i < layMax; i++)  // 18 floats, a b g r bytes, then ints
	{
		byte* l = base + layOff + 96 * i;
		for (int k = 0; k < 18; k++) SwapWord(l + 4 * k);
		for (int k = 0; k < 5; k++) SwapWord(l + 76 + 4 * k);
	}
	for (uint32_t i = 0; i < anmMax; i++)  // flag, index_sta, index_end, end_jmp, end_frm (+12 unused)
		for (int k = 0; k < 5; k++) SwapWord(base + anmOff + 32 * i + 4 * k);
	const uint64_t seqOff = (uint64_t)anmOff + 32ull * anmMax; // the game reads the steps here, not at seq_offset
	if (!inside(seqOff, seqMax, 16))
	{
		return false;
	}
	for (uint32_t i = 0; i < seqMax; i++)  // frm, cmd, para0, para1
		for (int k = 0; k < 4; k++) SwapWord(base + seqOff + 16 * i + 4 * k);
	return true;
}

string JetUtil::GetString(byte* src, int count)
{
	std::vector<byte> array(count);
	for (int i = 0; i < count; i++)
	{
		array[i] = src[i];
	}
	string text = string((const char*)array.data(), count);
	int length = CSharpCompat::IndexOf(text, '\0');
	return text.substr(0, length);
}

}
