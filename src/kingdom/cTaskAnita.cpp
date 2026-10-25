#include "kingdom/cTaskAnita.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"

namespace kingdom
{

cTaskAnita::cTaskAnita()
{
	AnitaVector = new cKingdomAnitaVector();
	nDrawPriority(2147483648u);
	SetName("cTaskAnita");
}

cTaskAnita::~cTaskAnita()
{
}

bool cTaskAnita::Execute(cJetTask* pParam)
{
	AnitaVector->UpdateFrame();
	return true;
}

void cTaskAnita::Draw(int nStatus, cJetTask* pParam)
{
	AnitaVector->Draw();
}

cKingdomAnita* cTaskAnita::pAnita(uint n)
{
	return AnitaVector->pAnita(n);
}

int cTaskAnita::nAnita()
{
	return AnitaVector->nAnita();
}

bool& cTaskAnita::bDrawReverse()
{
	return AnitaVector->bDrawReverse();
}

bool cTaskAnita::SetState(uint flag, bool bit)
{
	return AnitaVector->SetState(flag, bit);
}

}
