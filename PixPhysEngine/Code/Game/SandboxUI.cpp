#include "SandboxUI.hpp"

SandBoxUI::SandBoxUI()
{
}

SandBoxUI::~SandBoxUI()
{
}

void SandBoxUI::RenderMaterialBrushUI(SandboxPlayer* player)
{
}

void SandBoxUI::RenderMaterialSelector(SandboxPlayer* player)
{
}

void SandBoxUI::RenderBrushControls(SandboxPlayer* player)
{
}

void SandBoxUI::RenderMaterialProperties(CellMatType selectedMaterial)
{
}

ImVec4 SandBoxUI::GetMaterialColor(CellMatType matType)
{
	return ImVec4();
}

const char* SandBoxUI::GetPhysicsTypeName(PhyType physType)
{
	return nullptr;
}

const char* SandBoxUI::GetMaterialDescription(CellMatType matType)
{
	return nullptr;
}

void SandBoxUI::InitializeMaterialUIInfo()
{
}
