#pragma once

#include <string>
#include <vector>

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cObject.hpp"
#include "wii/Anim.hpp"

// Galaga Remix (g01) stage course: kingdom::game::g01::cTaskSceneBase (vtable 0x8043ba80) + cTaskScene (0x8043b9b8, 0x2b0 bytes; the boss stage
// uses cTaskBossScene). The "course" is the frame of the stage camera's SCN0 (m01s_s<N>_cam_01_sca); the scn file's command records set how fast
// it advances (speed ramps, waits, gates), its spawn records start enemies as the course passes them. Ported from main.dol with the old
// hand-reversed g01 tree (reversing/dol/g01_ghidra) and ~/Code/proj/unpac/kingdom/galaga (scn.py, stage.py) as guides.
namespace kingdom::game::g01
{
class cTaskEnemyMan;

// One record of a scn file (0x30 bytes, big endian on the disc)
struct sScnRecord
{
	float vParent[3];      // +0x00 parent translation of a spawn
	u32 n0c;               // +0x0c
	u32 nIndex;            // +0x10 list index (spawns run between the indexes of two commands)
	float fCourseTime;     // +0x14
	float fCourseSpeed;    // +0x18
	float fPacmanSpeed;    // +0x1c
	float fDelay;          // +0x20 local delay (waits, gates, spawns while the course stands)
	u32 nWord;             // +0x24 bits 30-31 class (0 command, 2 spawn); command: bits 0-14; spawn: bits 10-19 enemy type, 0-9 orbit
	u32 nParam1;           // +0x28 gate: the last formation group that has to be gone
	u32 nParam2;           // +0x2c command: bit 0 / 1 sound 0x21 / 0x22, bit 2 flag +0x240; spawn: bits 0-8 group
	u32 nClass() const { return nWord >> 30; }
	u32 nCommand() const { return nWord & 0x7fff; }
};
static_assert(sizeof(sScnRecord) == 0x30);

// DOL cScnData (0x18 bytes, vtable 0x8043918c): +0 record count, +4 command count, +0xc spawn count, +0x10 records
class cScnData
{
public:
	bool SetData(const u8* be, std::size_t size);   // byte order converted here
	std::vector<sScnRecord> records;
	u32 nCommand{};
	u32 nSpawn{};
};

class cTaskScene : public cObjectBase, public kingdom::cTaskKingdom, public cDemoInterface
{
public:
	// the opening demo of stage 1 (cDemoSceneW1S1, made by 0x800f6354) and the goal demos of stages 3 / 6 / 9 / 10
	void DemoStart() override { m_nState = 0; m_nStep = 0; m_bDemoDone = false; m_nDemoStep = 0; }   // DOL 0x800d201c
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override;          // cDemoSceneW1S1::Term 0x801c067c: done, then vtable +0x7c (the start camera 0x800f663c)
	void DemoGoalStart() override { m_nState = 1; m_nStep = 0; m_nGoalStep = 0; m_bGoalDone = false; }   // DOL 0x800d2034
	bool IsGoalDone() const override { return m_bGoalDone; }

	// DOL 0x800f6200 (Init after new 0x2b0): stage, enemy manager (natively set after construction: SetEnemyMan), base speed (app passes 1.0)
	cTaskScene(int nStage, void* pEnemyMan, float fSpeed);
	~cTaskScene() override;   // DOL 0x800d1dc8
	bool IsSegmentChanged() const { return m_bSegChanged; }   // +700 (cTaskBossScene: the next course segment started this frame)
	bool IsLooped() const { return m_bLooped; }               // +0x2bd (cTaskBossScene: the last segment restarted this frame)
	int nSegment() const { return m_nSeg; }                   // +0x2ac

	bool Execute(cJetTask* pParam) override;   // DOL 0x800f6900

	void ReadyStart() { m_nState = 3; m_nStep = 0; }       // DOL 0x800f6a80
	bool IsReady() const { return m_nState == 4; }          // DOL 0x800f6a6c
	void GameStart() { m_nState = 5; m_nStep = 0; }        // DOL 0x800f6aa8
	bool IsGame() const { return m_nState == 5; }
	bool b240() const { return m_b240; }        // +0x240: jumps are off           // DOL 0x800f6a94
	void MissStart() { m_nState = 6; m_nStep = 0; }        // DOL 0x800f6abc (Execute state 6: the course stops, rate 0)
	void GoalStart() { m_nState = 7; m_nStep = 0; }        // DOL 0x800f6ad0
	bool IsCourseEnd() const { return m_bEnd; }             // +0x1f8
	float fFrame() const { return m_fFrame < m_fLength ? m_fFrame : m_fLength; }   // vtable 0x1d8 slot 6 (native: clamped for the samples)
	float fRate() const { return m_fRate; }                 // DOL 0x800f7df8 (controller slot 8)
	float fPacmanSpeed() const { return m_fPacmanSpeed; }   // DOL 0x800f7e0c (+0x1f4)
	float fCourseLength() const { return m_fLength; }       // +0x1ec
	void ClearGroup(u32 nGroup) { if (nGroup < 128) m_nClearGroups[nGroup >> 5] |= 1u << (nGroup & 31); }   // DOL 0x800f8480
	void SetEnemyMan(cTaskEnemyMan* p) { m_pEnemyMan = p; }

protected:
	struct NoSetup {};
	cTaskScene(int nStage, float fSpeed, NoSetup);   // the members only: a derived scene (cTaskBossScene) does its own setup
	virtual void GameStep();           // vtable +0x74 (0x800f653c: CourseStep, UpdateFrame)
	void LoadCourse(int nCode);        // the course of another code (cTaskBossScene: stage + segment)
	void LoadCourse();                 // DOL 0x800f6fbc: m01s_s%x_cam_01_sca.brres; length = the longest SCN0
	void LoadLights();                 // DOL 0x800f7490: lig_01 -> light set 0, lig_c01 -> light set 1
	void SetStartCamera();             // DOL 0x800f663c (vtable +0x8c): m01s_%xstart_cam_sca.brres (world), rate 1
	void SetCourseCamera();            // DOL 0x800f66d0 (vtable +0x80): course camera + lights again + ResetCourse
	void SetDemoCamera();              // DOL 0x800f671c (vtable +0x84): m01s_s1_cam_10_sca + m01s_s1_lig_10_sca, frame 0, rate 1
	void SetGoalCamera();              // DOL 0x800f67a0 (vtable +0x88)
	void GoalCameraStep();             // DOL 0x800f6598 (vtable +0x78, state 7): m01s_s%x_cam_03_sca + the lights at frame 0, rate 1, then runs
	void ResetCourse();                // DOL 0x800f6bc4: first two commands, frame 0
	const sScnRecord* NextCommand();   // DOL 0x800f7784
	void CourseStep();                 // DOL 0x800f7e3c (vtable +0x4c)
	void UpdateFrame();                // DOL 0x800f7e28: the SCN0 frame controller advances by its rate
	void ApplyCamera();                // native: the g3d ScnRoot SetAnmScn camera -> kingdom::NativeScene
	void ApplyLights();                // native: the light SCN0s on the same controller (0x800196d4 -> 0x80018edc), sampled at its frame
	bool LoadScn0(const std::string& name);

	// cTaskSceneBase +0x4c (sub+0x10) = 0x800f84b0: the course scn data's +0xc (+0x234 -> +0xc), read through m_Scn natively
	int m_nStage{};                    // +0x1c8
	int m_nWorld{};                    // +0x1cc
	float m_fSpeed{1.0f};              // +0x1e8 base speed
	float m_fLength{};                 // +0x1ec
	float m_fTimer{};                  // +0x1f0 frames since the current command started
	float m_fPacmanSpeed{};            // +0x1f4
	bool m_bEnd{};                     // +0x1f8
	u32 m_nCmdNo{};                    // +0x22c
	u32 m_nCmdCursor{}, m_nCmdSeen{};  // +0x208 / +0x204 (NextCommand ring cursor)
	const sScnRecord* m_pCur{};        // +0x238
	const sScnRecord* m_pNext{};       // +0x23c
	u32 m_nNeedGroups[4]{};            // +0x21c..: formation groups that must be gone before a gate opens
	u32 m_nClearGroups[4]{};           // +0x20c..: groups whose enemies are all gone (shot or flown off)
	cTaskEnemyMan* m_pEnemyMan{};   // +0x1c4
	float m_fPrevTimer{}, m_fPrevFrame{};   // FLOAT_8047905c / FLOAT_80479058 (globals in the DOL)
	bool m_b240{};                     // +0x240
	kingdom::cKingdomPtkHnd m_CourseFx;   // +0x250 the course command effects (PTK_MAP_MIST01 / PTK_MAP_STONE01)
	bool m_bSegChanged{};              // +700
	bool m_bLooped{};                  // +0x2bd
	int m_nSeg{};                      // +0x2ac
	cScnData m_Scn;                    // +0x2a8 (new[] 1)
	// the animation controller +0x1d8 (frame / rate) over the SCN0 list +0x1dc
	std::vector<wii::ScnAnim> m_Anm;
	std::vector<std::vector<u8>> m_AnmData;
	float m_fFrame{}, m_fRate{};
	wii::ScnAnim m_LightAnm[2];        // lig_01 (light set 0, stage) / lig_c01 (1, characters); the demo's lig_10 replaces set 0
	wii::LightResult m_LightFog[2];    // the fog of each set (m01s_s%x_fog_01_sca, constant)
	bool m_bLight[2]{};
	bool m_bDemoDone{};                // the demo object's +8
	int m_nDemoStep{};                 // the demo object's state (0x801bce00)
	int m_nGoalStep{};                 // ... on the goal path (0x801bce94)
	bool m_bGoalDone{};                // the demo object's +9
};
}
