#pragma once

#include "common.hpp"
#include "ResUserData.hpp"
#include "wii/Mdl0.hpp"

class ResNode
{
public:
	Transform* This{};

	// native build: the node this stands for (the Unity Transform tree is scrapped); `This` carries the local pose for code that reads it
	const wii::Mdl0* Mdl{};
	int Idx{-1};
	ResNode(const wii::Mdl0* mdl, int idx);

	ResUserData* UserData{};

	ResNode();

	ResNode(Transform* baseNode);

	bool IsValid();

	bool IsActive();

	string GetName();

	ResNode* GetChildNode(int idx);

	ResUserData* GetResUserData();

	Vector4 GetTranslate(bool isLocal = true);

	Vector4 GetRotate(bool isLocal = true);

	Vector4 GetScale();

	Matrix4x4 GetTransRotMatrix(bool convert = true);

private:
	void SetNodeUserData(ResNode* node);
};
