#pragma once

#include "common.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cTaskModelBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskPushBack.hpp"

namespace kingdom::game::g03
{

class cTaskModelEx : public cTaskModelBase
{
public:
	class cPBModel : public cKingdomModel
	{
	private:
		cKingdomModel* m_pShadowModel{};

	public:
		cTaskPushBack* PushBack{};

		cKingdomModel* get_ShadowModel();

		bool IsUpdateFrame{};

		bool RegistShadowModel(string key);

		cPBModel();

		void Term() override;

		void IsDrawShadowDepth(bool b);

		bool IsDrawShadowDepth();

		void SetShadowModelMtx(Matrix4x4 m34, Vector4 scale);

		void UpdateShadowModelMtx();

		void SetMaterialColor(Color color);
	};

private:
	std::vector<cTaskModelEx::cPBModel*> m_vecPBModel;

public:
	cTaskModelEx();

	cTaskModelEx(string key, uint option = 0u);

	cTaskModelEx(Object* data, uint option = 0u);

private:
	void Init();

public:
	void Term() override;

	cKingdomModel* CreateModel(int idx = 0, bool isUpdateFrame = false);

	cKingdomModel* CreateModelEx(int& idx, bool isUpdateFrame = false);

	void DeleteModel(int idx = 0);

	cKingdomModel* pModel(int idx = 0);

	cTaskModelEx::cPBModel* pPBModel(int idx = 0);

	void UpdateFrame();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void SetMaterialColor(int n, Color color);
};

}
