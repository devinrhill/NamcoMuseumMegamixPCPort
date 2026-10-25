#include "kingdom/cKingdomPtkMan.hpp"

#include <cstdio>

#include "kingdom/ResourceDatabase.hpp"
#include "wii/Archive.hpp"

namespace kingdom
{

cKingdomPtkMan::~cKingdomPtkMan()
{
	Term();
}

void cKingdomPtkMan::Create(uint nSlot, uint nEmi, uint nGen, uint nPtc)
{
	cJetPtkMan::Create(nSlot, nEmi, nGen, nPtc);
}

void cKingdomPtkMan::Draw(Camera* camera, Vector4 vCamPos, Vector4 vCamTgt, float fFovy)
{
	cJetPtkMan::Draw(camera, vCamPos, vCamTgt, fFovy);
}

void cKingdomPtkMan::DebugDraw(int x, int y, uint w)
{
}

void cKingdomPtkMan::RegistData(uint nSlotNo, string pName)
{
	// TODO(unity): Debug.Log($"パーティクル登録 [{nSlotNo}]->{pName}");
	// archive entry name = the asset key (BIN_EFF_*)
	std::size_t size = 0;
	if (const std::uint8_t* d = wii::ArchiveStore::Find(pName, size))
	{
		std::vector<byte> bin(d, d + size);
		SetData(nSlotNo, bin);
	}
	else std::fprintf(stderr, "cKingdomPtkMan::RegistData(%u): %s not found\n", nSlotNo, pName.c_str());
	if ((pName.find("ef_steam") != string::npos))
	{
		SetOverrideAlphaTest(nSlotNo);
	}
}

void cKingdomPtkMan::RegistData(uint nSlotNo, uint nCRC)
{
	// TODO(unity): Debug.Log("[cKingdomPtk.RegistData] crc版が呼ばれた");
}

void cKingdomPtkMan::UpdateFrame(float fStep)
{
	if (fStep != 0.0f)
	{
		AnimUpdate(fStep);
	}
}

}
