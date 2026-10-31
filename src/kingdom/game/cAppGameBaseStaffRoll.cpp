#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

#include <cmath>
#include <cstdio>
#include <limits>

#include "cJetTask.hpp"
#include "CommonUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/NativeScene.hpp"
#include "KingdomGlobal.hpp"
#include "GamePad.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/cTaskStaffRollAnita.hpp"
#include "kingdom/game/cTaskStaffRollModel.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "MotosFrontUIManager.hpp"
#include <InGamePadManager.hpp>
#include <SoundManager.hpp>

namespace kingdom::game
{

void cAppGameBaseStaffRoll::Init()
{
	m_nStep = 0;
	m_nNextJobNo = std::numeric_limits<uint>::max();
	m_pBgmName = EBGMKind::BGM_001;
	KingdomGlobal::KingSys->AttachArcData("staffroll.lzs");   // DOL 0x801442a8 [audited: Unity render settings dropped] (museum/en/staffroll.lzs; the subclass Init attaches it again)
	// TODO(unity): m_prevAmbientMode = RenderSettings.ambientMode;
	// TODO(unity): m_prevLightColor = RenderSettings.ambientLight;
	// TODO(unity): m_prevSkyColor = RenderSettings.ambientSkyColor;
	// TODO(unity): m_prevEquatorColor = RenderSettings.ambientEquatorColor;
	// TODO(unity): m_prevGroundColor = RenderSettings.ambientGroundColor;
	// TODO(unity): m_light = Object.FindObjectOfType<UnityEngine.Light>();
	// TODO(unity): m_prevLightIntensity = m_light.intensity;
	// TODO(unity): m_prevLightRot = m_light.transform.eulerAngles;
	// TODO(unity): RenderSettings.ambientMode = AmbientMode.Flat;
	// TODO(unity): RenderSettings.ambientLight = new Color(0.65f, 0.5f, 0.5f);
	// TODO(unity): m_light.transform.eulerAngles = new Vector3(110f, -90f, 0f);
	// TODO(unity): m_light.intensity = 0.8f;
}

void cAppGameBaseStaffRoll::Term()
{
	if (nullptr != m_targetCam)
	{
		// TODO(unity): m_targetCam.enabled = false;
		// TODO(unity): m_targetCam.transform.position = m_prevCamPos;
		// TODO(unity): m_targetCam.transform.rotation = m_prevCamRot;
		// TODO(unity): m_targetCam.fieldOfView = m_prevCamFOV;
	}
	// TODO(unity): RenderSettings.ambientMode = m_prevAmbientMode;
	// TODO(unity): RenderSettings.ambientLight = m_prevLightColor;
	// TODO(unity): RenderSettings.ambientSkyColor = m_prevSkyColor;
	// TODO(unity): RenderSettings.ambientEquatorColor = m_prevEquatorColor;
	// TODO(unity): RenderSettings.ambientGroundColor = m_prevGroundColor;
	if (nullptr != m_light)
	{
		// TODO(unity): m_light.transform.eulerAngles = m_prevLightRot;
		// TODO(unity): m_light.intensity = m_prevLightIntensity;
	}
	MotosFrontUIManager::SetForceCameraEnable(false);
	if (m_pTaskBgMdl != nullptr)
	{
		m_pTaskBgMdl->TermOnce();
	}
	if (m_pTaskPacMdl != nullptr)
	{
		m_pTaskPacMdl->TermOnce();
	}
	if (m_pTaskGhostMdl != nullptr)
	{
		m_pTaskGhostMdl->TermOnce();
	}
	if (m_pTaskCookieMdl != nullptr)
	{
		m_pTaskCookieMdl->TermOnce();
	}
	KingdomGlobal::KingSys->DetachArcData("staffroll.lzs");   // DOL 0x801442ec (base dtor body): the port never detached it
}

void cAppGameBaseStaffRoll::Free()
{
	Term();
	cAppKingdom::Free();
}

// DOL 0x80144ec0 head: paused while HOME (0x400) is held on any of the 4 pads (the C# returned false)
bool cAppGameBaseStaffRoll::PauseCheck()
{
	for (int i = 0; i < 4; i++)
		if (GamePad::Get(i).hold & 0x400u) return true;
	return false;
}

void cAppGameBaseStaffRoll::RegistData()
{
	m_pTaskFade = new cTaskFade();
	m_pTaskFade->nDrawPriority(2415919103u);
	ConnectChild(m_pTaskFade);
	m_pTaskPicAnt = new cTaskStaffRollAnita();
	m_pTaskPicAnt->nDrawPriority(65537u);
	ConnectChild(m_pTaskPicAnt);
	m_pTaskTxtAnt = new cTaskStaffRollAnita();
	m_pTaskTxtAnt->nDrawPriority(2147483649u);
	ConnectChild(m_pTaskTxtAnt);
	m_pTaskBgMdl = new cTaskStaffRollModel();
	m_pTaskBgMdl->nDrawPriority(65538u);
	ConnectChild(m_pTaskBgMdl);
	m_pTaskPacMdl = new cTaskStaffRollModel();
	m_pTaskPacMdl->nDrawPriority(65539u);
	ConnectChild(m_pTaskPacMdl);
	m_pTaskGhostMdl = new cTaskStaffRollModel();
	m_pTaskGhostMdl->nDrawPriority(65540u);
	ConnectChild(m_pTaskGhostMdl);
	m_pTaskCookieMdl = new cTaskStaffRollModel();
	m_pTaskCookieMdl->nDrawPriority(65541u);
	ConnectChild(m_pTaskCookieMdl);
	SoundManager::get_BGM()->Stop(0.0f);
	SoundManager::get_BGM()->Play(m_pBgmName);
	RegistCamera("SCN_ANM_CAM_01");
	RegistPicAnt("ANT_STAFFROLL_02");
	RegistTxtAnt("ANT_STAFFROLL_01");
	RegistBgMdl("BG_MDL");
	RegistPacMdl("PACMAN_MDL", "PACMAN_ANM_CHA_01", "PACMAN_ANM_TPA_01");
	RegistGhostMdl("GHOST_MDL", "GHOST_ANM_CHA_01", "GHOST_ANM_TSA_01");
	RegistCookieMdl("COOKIE_MDL", "COOKIE_ANM_CHA_01", "COOKIE_ANM_CLA_01");
	// DOL 0x80144348 end: the light's far plane is set once more (62 instead of 60)
	kingdom::NativeScene::Shadow().SetOrtho(40.0f, 40.0f, 20.0f, 62.0f);
	// DOL: ScnRoot::SetAnmScn(SCN_ANM_CAM_01). nw4r CalcAnmScn then loads the view camera (camera 0), light set and fogs from it. Nothing calls
	// ScnRoot::UpdateFrame in this app (its only callers: the title, Gator Panic), so the scene stays at frame 0: a fixed camera. Dolphin: same view.
	{
		std::size_t size = 0;
		const std::uint8_t* d = wii::ArchiveStore::Find("SCN_ANM_CAM_01", size);
		wii::Brres b;
		wii::ScnAnim sa;
		wii::LightResult light;
		if (d != nullptr && b.Load(d, size) && sa.Load(b) && sa.SampleLight(0, light))
		{
			sa.SampleFog(light);
			for (int i = 0; i < 4; i++) kingdom::NativeScene::SetLight(i, light);
		}
		wii::CameraResult cam;
		if (d != nullptr && sa.SampleCamera("", 0.0f, cam))
		{
			float target[3];
			cam.LookAt(target);
			if (cam.projType == 0) kingdom::NativeScene::SetCamera(cam.pos, target, cam.fovy, cam.nearZ, cam.farZ);
			else kingdom::NativeScene::SetOrthoCamera(cam.pos, target, cam.fovy * 0.5f, cam.fovy * -0.5f, cam.fovy * -0.5f * cam.aspect, cam.fovy * 0.5f * cam.aspect, cam.nearZ, cam.farZ);
		}
		d = wii::ArchiveStore::Find("PACMAN_MDL", size);
		wii::Brres pb;
		wii::Mdl0 mdl;
		if (d != nullptr && pb.Load(d, size) && !pb.ModelNames().empty() && mdl.Load(pb, pb.ModelNames().front()) && mdl.nodes.size() > 1)
			m_pacRootNode = mdl.nodes[1].name;   // node 1 ("pacman"; 0 is the model root)
	}
	ConnectChild(new kingdom::cTaskNativeScene());
}

// DOL 0x80144348 [audited: the texture / model setup is the port's Regist* functions] (RegistData head): the scene root's cKingdomShadow (+0x1fc, camera 4, the DEPTH SHADOW LIGHT, not
// the view: the port used to draw the scene through it, straight from above): mode 0, target (0, 0, 0), rotation (pi/2, 0, 0), radius 40, ortho 40 x 40, near 20, far 60.
// The view camera is SCN_ANM_CAM_01 (RegistData).
void cAppGameBaseStaffRoll::RegistCamera(string DataKey)
{
	kingdom::NativeScene::SetShadow(true);   // +0x1f8 = 1
	kingdom::cKingdomShadow& sh = kingdom::NativeScene::Shadow();
	sh.SetMode(0);
	sh.SetTarget(0.0f, 0.0f, 0.0f);
	sh.SetAngles(1.5707964f, 0.0f, 0.0f);
	sh.SetOrtho(40.0f, 40.0f, 20.0f, 60.0f);
	sh.SetRadius(40.0f);
}

void cAppGameBaseStaffRoll::RegistPicAnt(string DataKey)
{
	m_pTaskPicAnt->pAnita(0u)->RegistData(ResourceDatabase::GetPath(DataKey));
	m_pTaskPicAnt->pAnita(0u)->SetAnimNo(0);
}

void cAppGameBaseStaffRoll::RegistTxtAnt(string DataKey)
{
	int drawLayer = 0; // TODO(unity): drawLayer = LayerMask.NameToLayer("FrontUI")
	MotosFrontUIManager::SetForceCameraEnable(true);
	m_pTaskTxtAnt->pAnita(0u)->RegistData(ResourceDatabase::GetPath(DataKey), drawLayer);
	m_pTaskTxtAnt->pAnita(0u)->SetAnimNo(0);
}

void cAppGameBaseStaffRoll::RegistBgMdl(string MdlKey)
{
	cKingdomModel* obj = m_pTaskBgMdl->CreateModel();
	obj->RegistModel(MdlKey);
	obj->AttachShadow();
	CommonUtil::SetLayer(obj->pScnMdl(), "Front3D");
}

void cAppGameBaseStaffRoll::RegistPacMdl(string MdlKey, string AnmKey, string AnmKey2)
{
	cKingdomModel* obj = m_pTaskPacMdl->CreateModel();
	obj->RegistModel(MdlKey);
	obj->RegistAnm(0u, AnmKey);
	obj->RegistAnm(0u, AnmKey2);
	obj->IsDrawShadowDepth(true);
	CommonUtil::SetLayer(obj->pScnMdl(), "Front3D");
}

void cAppGameBaseStaffRoll::RegistGhostMdl(string MdlKey, string AnmKey, string AnmKey2)
{
	cKingdomModel* obj = m_pTaskGhostMdl->CreateModel();
	obj->RegistModel(MdlKey);
	obj->RegistAnm(0u, AnmKey);
	obj->RegistAnm(0u, AnmKey2);
	obj->IsDrawShadowDepth(true);
	CommonUtil::SetLayer(obj->pScnMdl(), "Front3D");
}

void cAppGameBaseStaffRoll::RegistCookieMdl(string MdlKey, string AnmKey, string AnmKey2)
{
	cKingdomModel* obj = m_pTaskCookieMdl->CreateModel();
	obj->RegistModel(MdlKey);
	obj->RegistAnm(0u, AnmKey);
	obj->RegistAnm(0u, AnmKey2);
	obj->IsDrawShadowDepth(false);
	CommonUtil::SetLayer(obj->pScnMdl(), "Front3D");
}

cAppGameBaseStaffRoll::cAppGameBaseStaffRoll()
{
	Init();
}

void cAppGameBaseStaffRoll::pSetBgmName(EBGMKind pBgmName)
{
	m_pBgmName = pBgmName;
}

void cAppGameBaseStaffRoll::nSetNextJobNo(uint nNo)
{
	m_nNextJobNo = nNo;
}

bool cAppGameBaseStaffRoll::Execute(cJetTask* pParam)
{
	IsPause() = PauseCheck();
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);
			break;
		case 1u:
			if (m_pTaskFade->IsEnd())
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
			}
			break;
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			break;
		case 1u:
			if (m_pTaskPicAnt->pAnita(0u)->IsState(65536u) && m_pTaskTxtAnt->pAnita(0u)->IsState(65536u))
			{
				NextMode(n, 2u);
			}
			// DOL 0x80144ec0: any button of pad 0 but the D-pad and HOME (trigger word & 0xfffffbf0) skips; the C# only took Cancel
			if (GamePad::Get(0).trig & 0xfffffbf0u)
			{
				NextMode(n, 2u);
			}
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
			}
			break;
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
			SoundManager::get_BGM()->Stop();
			break;
		case 1u:
			if (m_pTaskFade->IsEnd())
			{
				Constants::Kingdom_SetJobNo(m_nNextJobNo, true);
				NextMode(n, 3u);
			}
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
			}
			break;
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (3 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			break;
		case 1u:
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
			}
			break;
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	// DOL 0x80144ec0 [audited+dolphin] end: the shadow light follows the translation of node 1 of Pac-Man's model (AnmObjChr::GetResult(result, 1)) every frame
	if (m_pTaskPacMdl != nullptr && !m_pacRootNode.empty())
	{
		Vector4 v;
		if (cKingdomModel* pac = m_pTaskPacMdl->pModel(0u)) pac->GetLocalNodeTrans(v, m_pacRootNode);
		kingdom::NativeScene::Shadow().SetTarget(v.x, v.y, v.z);
	}
	return true;
}

}
