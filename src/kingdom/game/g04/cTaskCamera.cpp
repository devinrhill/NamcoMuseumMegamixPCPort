#include "kingdom/game/g04/cTaskCamera.hpp"

#include <cmath>

#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

namespace kingdom::game::g04
{

// DOL 0x8019d314 [audited]
cTaskCamera::cTaskCamera()
{
	SetName("g04::cTaskCamera");
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_CAMERA"))   // cParamCamera
	{
		m_fShakeRow = csv.F32(0, 1, 0.2f);
		m_fShakeCol = csv.F32(1, 1, 0.1f);
		m_fShakeSub = csv.F32(2, 1, 0.02f);
		m_fShakeSin = csv.F32(3, 1, 30.0f);   // "30.0f" in the CSV
	}
	kingdom::cKingdomCSV prm;
	if (prm.RegistData("CSV_CAMERA_PARAM"))   // cParamCameraSetting
		for (int i = 0; i < 4; i++)
		{
			m_fZoom[i] = prm.F32(i, 1, float(m_fZoom[i]));   // an rvalue: an lvalue picks the "out" overload (returns 1.0)
			m_fGame[i] = prm.F32(i + 4, 1, float(m_fGame[i]));
		}
	SetMode(0);
}

// DOL 0x80094ba8 [audited: camera applied through NativeScene::SetCamera]: camera 0 of the SCN0 at frame 0: its eye / aim, the projection the caller set (0x80095030)
bool cTaskCamera::TakeSceneCamera(const char* sca, float fovy, float nearZ, float farZ)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(sca, size);
	wii::Brres b;
	wii::ScnAnim a;
	wii::CameraResult cam;
	if (d == nullptr || !b.Load(d, size) || !a.Load(b) || !a.SampleCamera("", 0.0f, cam)) return false;
	for (int i = 0; i < 3; i++)
	{
		m_fEye[i] = cam.pos[i];
		m_fTarget[i] = cam.aimPos[i];
	}
	m_fFovy = fovy;
	m_fNear = nearZ;
	m_fFar = farZ;
	m_nCamMode = 1;
	Apply();
	return true;
}

// DOL 0x8019d724 [audited]
void cTaskCamera::SetMode(int mode)
{
	if (mode == 0) TakeSceneCamera("BG_SCA", m_fGame[3], m_fGame[0], m_fGame[1]);   // 0x3e9a6c40
	else if (mode == 1)
	{
		m_nCamMode = 3;
		m_fRow = m_fShakeRow;
		m_fCol = m_fShakeCol;
		m_fStep = m_fShakeSin;
		m_fSub = m_fShakeSub;
	}
	else if (mode == 2)
		TakeSceneCamera("BG_ZOOM_SCA", m_fZoom[3], m_fZoom[0], m_fZoom[1]);   // 0x3bfdafbe
}

// DOL 0x80094e2c [audited: shake math compared (sin of 0.01745*t, row/col decay)]
void cTaskCamera::Apply()
{
	if (m_nCamMode == 1)
		kingdom::NativeScene::SetCamera(m_fEye, m_fTarget, m_fFovy, m_fNear, m_fFar);
	else if (m_nCamMode == 3)
	{
		m_fT += m_fStep;
		if (360.0f <= m_fT) m_fT -= 360.0f;   // FLOAT_8047abac
		const float s = std::sin(0.017453292f * m_fT);   // FLOAT_8047abb0
		const float eye[3] = {m_fEye[0], m_fRow * s + m_fEye[1], m_fEye[2]};
		const float target[3] = {m_fCol * s + m_fTarget[0], m_fTarget[1], m_fTarget[2]};
		kingdom::NativeScene::SetCamera(eye, target, m_fFovy, m_fNear, m_fFar);
		m_fCol -= m_fSub;
		m_fRow -= m_fSub;
		if (m_fRow <= 0.0f) m_fRow = 0.0f;
		if (m_fCol <= 0.0f) m_fCol = 0.0f;
	}
}

// DOL 0x8019d630 [audited]
bool cTaskCamera::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app != nullptr && app->IsPause()) return true;
	if (m_nTaskMode == 1 && m_nCamMode == 3 && m_fCol == 0.0f && m_fRow == 0.0f) SetMode(0);
	Apply();
	return cTaskKingdom::Execute(pParam);
}

}
