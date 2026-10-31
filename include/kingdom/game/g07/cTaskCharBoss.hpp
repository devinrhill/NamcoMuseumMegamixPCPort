#pragma once

#include "kingdom/NativeScene.hpp"

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBomb.hpp"

namespace kingdom::game::g07
{

class cTaskCharBoss : public cTaskCharEnemy
{
private:
	int m_nHp{};

	int m_nMaxHp{};

	float m_fChargeDamageRate{};

	int m_nMovePoint{};

	int m_nAction{};

	int m_nScore{};

	int m_nExplodeTimer{};

	float m_fHitQuake{};

	int m_nHitQuakeTime{};

	int m_nBombMax{};

	int m_nBombTime{};

	cTaskCharEnemyBomb* m_pSpawnBomb[32]{};

	uint m_nApearDemoTimer{};

	float m_fFogNearTmp{};

	float m_fFogFarTmp{};

	uint m_nFogColorTmp{};

	// the boss flash fog (ScnRoot fog slot 3 = SCA_BOSS_HIT): the live copy (+0xa40: type, start, end, near, far, colour) and its backup (+0xa58)
	kingdom::NativeScene::FogSlot m_fogLive{};
	kingdom::NativeScene::FogSlot m_fogBack{};

	uint m_nApearDemoTimerTmp{};

	float m_fCoreFogNear{};

	float m_fCoreFogFar{};

	uint m_nCoreFogColor{};

	ushort m_nReturnStep{};

	bool m_bAppearDemoEnd{};

public:
	static constexpr int nFRASH_RATE = 8;

	static constexpr int nFADE_SPD_RATE = 1000;

	static constexpr int nFADE_STOP_TIME = 60;

private:
	bool m_nowFramePlaySEHitZako01{};

	bool m_nowFramePlaySEHitBoss01{};

protected:
	static const int scEmissionColorPropertyID;

	static const Color scFlashColor;

	std::vector<std::vector<Material*>> m_flashMatLists;

	std::vector<std::vector<Color>> m_baseMatColorLists;

private:
	float near_{};

	float far_{};

public:
	cTaskCharBoss();

	bool TaikiAction() override;

	int& nHp();

	int& nMaxHp();

	float& fChargeDamageRate();

	int& nScore();

	uint& nApearDemoTimer();

	float& fHitQuake();

	int& nHitQuakeTime();

	int& nBombMax();

	int& nBombTime();

	cTaskCharEnemyBomb* pSpawnBomb(int n);

	float& fFogNear();

	float& fFogFar();

	float& fCoreFogNear();

	float& fCoreFogFar();

	uint& nCoreFogColor();

	int& nExplodeTimer();

	void ReturnStep(ushort step);

	ushort ReturnStep();

	int& nAction();

	int& nMovePoint();

	void StepInit();

	bool IsApearDemoEnd();

	bool& bApearDemoEnd();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void PresetEnv() override;

	void RegistPacket() override;

	void SyncPacket() override;

	void CreateBomb();

	cTaskCharEnemyBomb* SpawnBombRot(float rot, float range, float fsp, float nohit_range, float offset = 1.0f);

	cTaskCharEnemyBomb* SpawnBomb(int dir, float range, float fsp, float nohit_range, float offset = 1.0f);

	cTaskCharEnemyBomb* GetTaikiBomb();

	void BombAllExplode();

	virtual void SetShadow(uint kind);

	virtual void SpawnBossColEffect(cTaskChar* pChar, int kind = -1, int chip = -1);

	virtual void SpawnBossColEffect_Core(cTaskChar* pChar, int kind = -1, int chip = -1);

	virtual void SpawnAppearEffect(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int nKind = -1);

	virtual void SpawnAppearEffectCircle(cTaskChar* pchar, Vector4 porg, float lr, float space);

	virtual void SpawnExplode(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float lz, float space, int delay = 0);

	virtual void SpawnExplode(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int delay = 0);

	virtual void SpawnExplodeRandom(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float lz, float space, int rand_span, int delay = 0);

	virtual void SpawnExplodeRandom(cTaskChar* pchar, Vector4 porg, float lx, float lz, float space, int rand_span, int delay = 0);

	virtual void SpawnExplode2(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float ly, float space, int delay = 0);

	virtual void SpawnExplode2(cTaskChar* pchar, Vector4 porg, float lx, float ly, float space, int delay = 0);

	virtual void SpawnExplode2Random(Vector4 trans, Vector4 rot, Vector4 porg, float lx, float ly, float space, int rand_span, int delay = 0);

	virtual void SpawnExplode2Random(cTaskChar* pchar, Vector4 porg, float lx, float ly, float space, int rand_span, int delay = 0);

	float IsAdjustRotate(float rot, float arot, float add);

	int HitDamage(cTaskChar* pChar, const int* pHitDamageTbl);

	void ExplodeSE(uint nRandMin = 5u, uint nRandMax = 18u);

	bool BossFadeAnitaDraw(cJetTask* pTask, cJetTask* pList, cJetTask* pScene);

	void SetHitQuake(int time);

	void GetFogData();

	void InitFogData();

	// DOL 0x801ea3c8 (vtable +0x13c): fog end += speed, slot 3 = linear start / end; 0x801ea4b0 (+0x140): fog end -= speed
	void FogStep(float speed);
	void FogStepBack(float speed);

protected:
	void SetFlash(int modelIdx, float rate);

protected:
	void ReleaseTerm() override { cTaskCharBoss::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
