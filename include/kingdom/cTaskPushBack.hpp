#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom
{

class cTaskPushBack : public cTaskKingdom
{
private:
	bool m_bClipping{};

	int m_nDrawCamID{};

	uint m_nPushBackNum{};

public:
	bool get_IsClipping();
	void set_IsClipping(bool value);

	int get_nDrawCamID();
	void set_nDrawCamID(int value);

private:
	void Init();

	void Term();

public:
	cTaskPushBack();

	virtual ~cTaskPushBack();

	bool PostExecute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	virtual void DrawScnRootOpa();

	virtual void DrawScnRootXlu();

	virtual Vector4 vClipTrans();

	virtual float fClipFovy();

	virtual bool IsDrawShadowDepth();

	// registers this task's models with the 3D pass for this frame (native: NativeScene); the C# queued them in the g3d scene
	bool PushBack();

protected:
	virtual void SubmitNative() {}

public:
};

}
