#pragma once

#include "common.hpp"
#include "RMDLUserDataParam.hpp"
#include "ResNode.hpp"
#include "wii/Mdl0.hpp"

class ResMdl
{
public:
	GameObject* This{};
	const wii::Mdl0* Mdl{};   // native build: the model's parsed MDL0 (null without a model)

	RMDLUserDataParam* UserDataRoot{};

	ResMdl();

	ResMdl(GameObject* baseObj);

	bool IsValid();

	ResNode* GetResNode(string nodeName);

	ResNode* GetResNode(int idx);

	ResNode* GetFirstChildResNode(int idx);

	ResNode* GetFirstChildChildResNode(int idx);

	int GetResNodeNumEntries();

	int GetChildResNodeNumEntries();

private:
	void SetNodeUserData(ResNode* node);
};
