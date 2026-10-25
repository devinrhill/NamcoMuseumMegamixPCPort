#pragma once

#include "common.hpp"
#include "jet/uDatAnmP.hpp"
#include "jet/uDatLayP.hpp"
#include "jet/uDatPat.hpp"
#include "jet/uDatSeqP.hpp"
#include "jet/uDatTex.hpp"

namespace jet
{

class JetUtil
{
public:
	static uDatTex* anpTexPtr(void* ptr, int n);

	static int anpTexCount(void* ptr);

	static string anpTexTexName(void* ptr, int n);

	static uDatPat* anpPatPtr(void* ptr, int n);

	static int anpPatCount(void* ptr);

	static int anpPatLayCount(void* ptr, int n);

	static uDatLayP* anpLayPtr(void* ptr, int n);

	static uDatLayP* anpPatLayPtr(void* ptr, int p, int l);

	static uDatAnmP* anpAnmPtr(void* ptr, int n);

	static int anpAnmCount(void* ptr);

	static int anpAnmSeqCount(void* ptr, int n);

	static uDatSeqP* anpAnmSeqPtr(void* ptr, int n, int s);

	static uDatSeqP* anpAnmSeqBSPtr(void* ptr, int n, float f);

	// The disc's Anita files are big-endian ("ANP 150 ", all ints/floats). The accessors above overlay host structs on
	// the bytes, so SetData converts the blob to host order once. No-op if it is already in host order.
	static bool anpToHostEndian(std::vector<byte>& data);

	static string GetString(byte* src, int count);
};

}
