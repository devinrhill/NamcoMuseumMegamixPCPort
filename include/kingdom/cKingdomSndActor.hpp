#pragma once

#include "common.hpp"

namespace kingdom
{
// cKingdomSndActor (DOL vtable 0x80393748): a 4-byte holder of an nw4r Sound3DActor (0x80 bytes, made by 0x8002654c on the sound manager
// KingSys +0xa54). Galaga's cGalagaSndActor (vtable 0x8044b0b8, factory 0x801c1680) only adds an id from DAT_80479110. Natively the actor is a
// position the SE bank places the actor's cues at (SoundSeBank::PlayActor).
class cKingdomSndActor
{
public:
	cKingdomSndActor() = default;   // DOL 0x8002654c
	~cKingdomSndActor();            // DOL 0x800265c8
	cKingdomSndActor(const cKingdomSndActor&) = delete;
	cKingdomSndActor& operator=(const cKingdomSndActor&) = delete;
	void SetPosition(const Vector4& pos);   // DOL 0x80026620
	void SetPosition(const float* pos);
	void ResetVelocity() {}                 // DOL 0x80026660 (velocity (0, 0, 0): no Doppler natively)
	// DOL 0x80025988 with the actor as the startable: returns the mixer voice (-1 = nothing)
	int Play(const char* cue);

private:
	Vector3 m_pos{};
};
}
