#include "kingdom/cKingdomAnitaVector.hpp"

#include "kingdom/cKingdomAnita.hpp"

namespace kingdom
{

void cKingdomAnitaVector::Init()
{
	m_vecAnita.reserve(16);
	m_bDrawReverse = false;
}

void cKingdomAnitaVector::Term()
{
	for (int i = 0; i < (int)m_vecAnita.size(); i++)
	{
		m_vecAnita[i]->Term();
		m_vecAnita[i] = nullptr;
	}
}

cKingdomAnitaVector::cKingdomAnitaVector()
{
	Init();
}

cKingdomAnitaVector::~cKingdomAnitaVector()
{
	Term();
}

cKingdomAnita* cKingdomAnitaVector::pAnita(uint n)
{
	if ((long)n >= (long)(int)m_vecAnita.size())
	{
		int num = (int)n - (int)m_vecAnita.size() + 1;
		for (int i = 0; i < num; i++)
		{
			m_vecAnita.push_back(new cKingdomAnita());
		}
	}
	if (m_vecAnita[(int)n] == nullptr)
	{
		m_vecAnita[(int)n] = new cKingdomAnita();
	}
	return m_vecAnita[(int)n];
}

int cKingdomAnitaVector::nAnita()
{
	return (int)m_vecAnita.size();
}

bool& cKingdomAnitaVector::bDrawReverse()
{
	return m_bDrawReverse;
}

bool cKingdomAnitaVector::SetState(uint flag, bool bit)
{
	bool result = true;
	cKingdomAnita* cKingdomAnita2 = nullptr;
	for (int i = 0; i < (int)m_vecAnita.size(); i++)
	{
		cKingdomAnita2 = m_vecAnita[i];
		if (cKingdomAnita2 != nullptr && !cKingdomAnita2->GetData().empty() && !cKingdomAnita2->SetState(flag, bit))
		{
			result = false;
			break;
		}
	}
	return result;
}

void cKingdomAnitaVector::UpdateFrame()
{
	cKingdomAnita* cKingdomAnita2 = nullptr;
	for (int i = 0; i < (int)m_vecAnita.size(); i++)
	{
		cKingdomAnita2 = m_vecAnita[i];
		if (cKingdomAnita2 != nullptr && !cKingdomAnita2->GetData().empty())
		{
			cKingdomAnita2->UpdateFrame();
		}
	}
}

void cKingdomAnitaVector::Draw()
{
	if (!bDrawReverse())
	{
		cKingdomAnita* cKingdomAnita2 = nullptr;
		for (int i = 0; i < (int)m_vecAnita.size(); i++)
		{
			cKingdomAnita2 = m_vecAnita[i];
			if (cKingdomAnita2 != nullptr && !cKingdomAnita2->GetData().empty())
			{
				cKingdomAnita2->Draw();
			}
		}
		return;
	}
	cKingdomAnita* cKingdomAnita3 = nullptr;
	for (int num = (int)m_vecAnita.size() - 1; num >= 0; num--)
	{
		cKingdomAnita3 = m_vecAnita[num];
		if (cKingdomAnita3 != nullptr && !cKingdomAnita3->GetData().empty())
		{
			cKingdomAnita3->Draw();
		}
	}
}

}
