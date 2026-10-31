#include "kingdom/game/g01/cTaskEnemySpikeMaster.hpp"

#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "snd/Mixer.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr uint SPIKE_HIT01 = 0x12;   // PTK_SPIKE_HIT01
constexpr uint SPARK = 0x1c;         // PTK_M_COMET01
}

cTaskEnemySpikeMaster::cTaskEnemySpikeMaster()
{
	SetName("g01::cTaskEnemySpikeMaster");
}

// DOL 0x800e8e78 [audited]: the spike's setup (0x800e8310), active, not collidable, type 13 kept (+0x25c = 0xd), grow 0; the placement +0xcc =
// 0x800e9000 (model type 14 with fog 3, the node's SRT less the model's root offset: as the spike's 0x800e8514); SpikeMasterGalaga_s%x.csv
// row nIndex columns 1..3 (+0x364)
void cTaskEnemySpikeMaster::Setup(const wii::Mdl0Node& node, int nStage, int nIndex)
{
	SetupSpike(node, nStage, nIndex, 14, "SpikeMasterGalaga_s%x.csv");   // s_SpikeMasterGalaga_s_x_csv_803738f4
	m_nType = 13;
	m_nGrow = 0;
}

// DOL 0x800e9420 [audited]: grow 0: CHAR_ENEMY_SPIKEMASTER_ANM_CHA_04 / CLA_04 at rate 1; grow 1 at the bone clip's end (natively "<="): CHA_01 and
// TSA_01 at rate 1, frame 0
void cTaskEnemySpikeMaster::MasterGrowStep()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	if (m_nGrow == 0)
	{
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKEMASTER_ANM_CHA_04");   // 0xeadcc761
		m->SetUpdateRateAnmChr(0u, 1.0f);                        // FLOAT_8047b57c
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKEMASTER_ANM_CLA_04");   // 0x1f5c61a1
		m->SetUpdateRateMatClr(0u, 1.0f);
		m_nGrow = 1;
	}
	else if (m_nGrow == 1 && m->GetFrameMaxAnmChr(0u) <= m->GetFrameAnmChr(0u))
	{
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKEMASTER_ANM_CHA_01");   // 0x9ab633ee
		m->SetUpdateRateAnmChr(0u, 1.0f);
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKEMASTER_ANM_TSA_01");   // 0x9355a75e
		m->SetUpdateRateAnmTexSrt(0u, 1.0f);
		m->SetFrameAnmChr(0u, 0.0f);
		m_nGrow = 2;
	}
}

// DOL 0x800e9574 [audited] (through the enemy wrapper 0x800e4704: active, not paused, the miss freeze): the clips, the model, the world box; while the
// course runs: state 0 -> state 1 at the action frame (+0xbc), state 1 grows (+0x7c), state 2 leaves (+0x84); collidable from the
// "collision from" frame on (never off again; no shoot marker); the hit flash step (+0x88, fog not drawn natively)
bool cTaskEnemySpikeMaster::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800afe2c
	UpdateFrame();   // FUN_8009a478
	cTaskModel::Execute(pParam);
	UpdateWorldAABB();   // FUN_801c1950
	cTaskScene* scene = cTaskEnemyMan::s_pInstance->pScene();
	if (scene == nullptr || !scene->IsGame()) return true;
	const float frame = scene->fFrame();
	if (m_nState == 0)
	{
		if (m_fAction < frame)
		{
			m_nState = 1;
			m_nStep = 0;
			static int tick = 0;
			if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 spike master grows at course frame %.1f, pos (%.1f %.1f %.1f) [call %d]\n", frame, m_v20.x, m_v20.y, m_v20.z, ++tick);
		}
	}
	else if (m_nState == 1)
		MasterGrowStep();
	else if (m_nState == 2)
		Leave();   // +0x84 (a master is never killed); the flash step still follows
	if (!m_b14 && m_nState != 2 && m_fHitFrom < frame) m_b14 = true;
	if (0.0f < m_fFlash) m_fFlash -= 1.0f;
	return true;
}

// DOL 0x800e9710 [audited]: Pac-Man (kind 3): PTK_SPIKE_HIT01 at it; a player's shot (kind 1): PTK_M_COMET01 and m01se_03hit1 (DAT_8047b570); no damage
void cTaskEnemySpikeMaster::HitCallback(cObject* other)
{
	const Vector4 pos(m_v20.x, m_v20.y, m_v20.z, 1.0f);
	if (other->nKind() == 3)
		(new cKingdomPtkHnd())->Generate(SPIKE_HIT01, true, false, true, 0.0f, pos);
	else if (other->nKind() == 1)
	{
		(new cKingdomPtkHnd())->Generate(SPARK, true, false, true, 0.0f, pos);
		snd::Play("m01se_03hit1");
	}
}

}
