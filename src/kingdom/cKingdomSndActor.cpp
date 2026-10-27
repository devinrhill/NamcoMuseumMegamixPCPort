#include "kingdom/cKingdomSndActor.hpp"

#include "SoundManager.hpp"

namespace kingdom
{
// DOL 0x800265c8
cKingdomSndActor::~cKingdomSndActor()
{
	SoundManager::get_SE()->ForgetActor(this);
}

// DOL 0x80026620
void cKingdomSndActor::SetPosition(const Vector4& pos)
{
	m_pos = Vector3(pos.x, pos.y, pos.z);
	SoundManager::get_SE()->SetActorPosition(this, m_pos);
}

void cKingdomSndActor::SetPosition(const float* pos)
{
	SetPosition(Vector4(pos[0], pos[1], pos[2], 1.0f));
}

// DOL 0x80025988 (the actor given)
int cKingdomSndActor::Play(const char* cue)
{
	return SoundManager::get_SE()->PlayActor(cue, this, m_pos);
}
}
