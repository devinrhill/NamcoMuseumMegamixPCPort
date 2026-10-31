#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include <EBGMKind.hpp>
#include "cJetTask.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/cTaskStaffRollAnita.hpp"
#include "kingdom/game/cTaskStaffRollModel.hpp"

namespace kingdom::game
{

class cAppGameBaseStaffRoll : public cAppKingdom
{
private:
	enum EPri : uint
	{
		ePRIORITY_PIC_ANT = 65537u,
		ePRIORITY_BG_MDL = 65538u,
		ePRIORITY_PAC_MDL = 65539u,
		ePRIORITY_GHOST_MDL = 65540u,
		ePRIORITY_COOKIE_MDL = 65541u,
		ePRIORITY_SCNROOT = 2147483647u,
		ePRIORITY_TXT_ANT = 2147483649u,
		ePRIORITY_FADE = 2415919103u,
		ePRIORITY_MAX = 2415919104u
	};

	enum EStat
	{
		eSTATE_FADE_IN = 0,
		eSTATE_MAIN = 1,
		eSTATE_FADE_OUT = 2,
		eSTATE_END = 3
	};

	int m_nStep{};

	uint m_nNextJobNo{};

	EBGMKind m_pBgmName{};

	cTaskFade* m_pTaskFade{};

	cTaskStaffRollAnita* m_pTaskPicAnt{};

	cTaskStaffRollAnita* m_pTaskTxtAnt{};

	cTaskStaffRollModel* m_pTaskBgMdl{};

	cTaskStaffRollModel* m_pTaskPacMdl{};

	cTaskStaffRollModel* m_pTaskGhostMdl{};

	cTaskStaffRollModel* m_pTaskCookieMdl{};

	Camera* m_targetCam{};

	Vector3 m_prevCamPos{};

	Quaternion m_prevCamRot{};

	float m_prevCamFOV{};

	AmbientMode m_prevAmbientMode{};

	Color m_prevLightColor{};

	Color m_prevSkyColor{};

	Color m_prevEquatorColor{};

	Color m_prevGroundColor{};

	Vector3 m_prevLightRot{};

	float m_prevLightIntensity{};

	Light* m_light{};

	string m_pacRootNode;


	void Init();

	void Term();

public:
	void Free() override;

	bool PauseCheck();

	void RegistData();

	void RegistCamera(string DataKey);

	void RegistPicAnt(string DataKey);

	void RegistTxtAnt(string DataKey);

	void RegistBgMdl(string MdlKey);

	void RegistPacMdl(string MdlKey, string AnmKey, string AnmKey2);

	void RegistGhostMdl(string MdlKey, string AnmKey, string AnmKey2);

	void RegistCookieMdl(string MdlKey, string AnmKey, string AnmKey2);

	cAppGameBaseStaffRoll();

	void pSetBgmName(EBGMKind pBgmName);

	void nSetNextJobNo(uint nNo);

	bool Execute(cJetTask* pParam) override;
};

}
