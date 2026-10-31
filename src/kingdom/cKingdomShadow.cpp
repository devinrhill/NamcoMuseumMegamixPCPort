#include "kingdom/cKingdomShadow.hpp"

#include <cmath>

namespace kingdom
{
// DOL 0x800241e0 [audited]: camera 0 (later 4), mode 1, target (0, 0, 0), radius 0
void cKingdomShadow::Reset()
{
	*this = cKingdomShadow();
}

// DOL 0x800244dc [audited: mode 1 and the twist are not ported], mode 0
bool cKingdomShadow::GetCamera(float eye[3], float target[3], float box[4], float& zNear, float& zFar) const
{
	if (!m_bOrtho || m_nMode != 0) return false;
	const float rx = m_fAngle[0], ry = m_fAngle[1];
	const float flat = m_fRadius * std::cos(rx);
	eye[0] = std::sin(ry) * flat + m_vTarget[0];
	eye[1] = m_fRadius * std::sin(rx) + m_vTarget[1];
	eye[2] = std::cos(ry) * flat + m_vTarget[2];
	// sin(90 deg) in float leaves x / z at ~1e-6: nw4r tests == 0 for its straight-down case, snap like the Wii's exact values would
	if (std::fabs(eye[0] - m_vTarget[0]) < 1e-4f) eye[0] = m_vTarget[0];
	if (std::fabs(eye[2] - m_vTarget[2]) < 1e-4f) eye[2] = m_vTarget[2];
	for (int i = 0; i < 3; i++) target[i] = m_vTarget[i];
	box[0] = m_fWidth * 0.5f; box[1] = -m_fWidth * 0.5f; box[2] = -m_fHeight * 0.5f; box[3] = m_fHeight * 0.5f;
	zNear = m_fNear; zFar = m_fFar;
	return true;
}
}
