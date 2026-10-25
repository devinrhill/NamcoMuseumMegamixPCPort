#include "kingdom/game/g02/cTaskRallyAnitaBase.hpp"

#include "GamePad.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"

namespace kingdom::game::g02
{

// the inlined base constructor of every part (e.g. in cTaskConsFuel 0x8006210c): cTaskAnita (priority 0x80000000, name "cTaskAnita"), then
// priority 0x80000001 and +0x1c8 = 1
cTaskRallyAnitaBase::cTaskRallyAnitaBase(cTaskConsole* pConsole) : m_pConsole(pConsole)
{
	SetName("g02::cTaskRallyAnitaBase");
	nDrawPriority(0x80000001u);
	m_b1c8 = true;
}

// DOL 0x80122f3c: unless HOME is held, the layouts step when the game is not paused (app +0x1b4), not at the stage end (app +0x284) and HOME
// is not held (the test is repeated, as in the DOL). pParam is the app in the DOL; the port asks the console (which holds the app).
bool cTaskRallyAnitaBase::Execute(cJetTask* pParam)
{
	(void)pParam;
	if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0)
	{
		if (m_pConsole == nullptr || !m_pConsole->IsFrozen()) AnitaVector->UpdateFrame();   // 0x80014d58
	}
	return true;
}

// DOL 0x80123244: an orthographic projection of the render mode's size (-w/2 .. w/2, -h/2 .. h/2 with w, h * 0.5 = FLOAT_8047bfa4; near -1 /
// far 1 = FLOAT_8047bfa8 / FLOAT_8047bfac) around the anita vector draw (0x80014df8). The port's anita draw sets up its own 640x480 projection.
void cTaskRallyAnitaBase::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
}

// DOL 0x80123034: RegistData (the archive entry by CRC, 0x8000bca0 / 0x80141708), SetAnimNo(anim, reset = true, frame 0), vTrans = (x, y, z)
kingdom::cKingdomAnita* cTaskRallyAnitaBase::Regist(uint idx, const char* name, int anim, float x, float y, float z)
{
	kingdom::cKingdomAnita* a = pAnita(idx);
	a->RegistData(name);
	a->SetAnimNo(anim, true, 0.0f);
	a->vTrans()[0] = x;
	a->vTrans()[1] = y;
	a->vTrans()[2] = z;
	return a;
}

// DOL 0x80122ff4: SetAnimNo(anim, reset = false, frame 0)
void cTaskRallyAnitaBase::SetAnim(uint idx, int anim)
{
	pAnita(idx)->SetAnimNo(anim, false, 0.0f);
}

// DOL 0x80123138: 0x80014408(anita, &pos, lay, pat); on success vTrans.x / y = pos.x / y
bool cTaskRallyAnitaBase::SetPosLayer(uint idx, int pat, uint lay)
{
	Vector4 pos{};
	kingdom::cKingdomAnita* a = pAnita(idx);
	if (!a->GetLayerPos(pos, lay, pat)) return false;
	a->vTrans()[0] = pos[0];
	a->vTrans()[1] = pos[1];
	return true;
}

// DOL 0x801231e8
bool cTaskRallyAnitaBase::GetLayerPos(Vector4& out, uint idx, int pat, uint lay)
{
	return pAnita(idx)->GetLayerPos(out, lay, pat);
}

bool cTaskRallyAnitaBase::IsEnd(uint idx)
{
	return pAnita(idx)->IsState(0x10000u);
}

void cTaskRallyAnitaBase::Show(uint idx, bool b)
{
	pAnita(idx)->SetState(1u, b);
}

void cTaskRallyAnitaBase::Animate(uint idx, bool b)
{
	pAnita(idx)->SetState(2u, b);
}

}
