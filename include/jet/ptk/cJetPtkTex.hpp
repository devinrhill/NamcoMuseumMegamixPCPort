#pragma once

#include "common.hpp"
#include "jet/ptk/PtkTex.hpp"

namespace jet::ptk
{

class cJetPtkTex
{
private:
	std::vector<PtkTex*> m_pTexs;

	uint m_nCur{};

	uint m_nMax{};

	std::unordered_map<std::string, PtkTex*> m_Hash;

	static string ssRootPath;

public:
	static string get_RootPath();

	static void set_RootPath(string value);

private:
	void Init();

	void Term();

	void Regist(Texture* pBin, string pName);

	void TexNameConv(string& pDst, string pSrc);

public:
	cJetPtkTex();

	cJetPtkTex(uint nTexCnt);

	virtual ~cJetPtkTex();

	void Create(uint nTexCnt);

	void Release();

	void Regist(std::vector<byte>& data);

	PtkTex* Search(string name);
};

}
