// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArthurSwordVisuals.h"

#include "Engine/StaticMesh.h"

namespace
{
	const TCHAR* ArthurSwordMeshPath = TEXT("/Game/VFX/Projectiles/Hero1/Arthur_Sword.Arthur_Sword");
}

const TCHAR* T66ArthurSwordVisuals::GetSwordMeshAssetPath()
{
	return ArthurSwordMeshPath;
}

UStaticMesh* T66ArthurSwordVisuals::LoadSwordMesh()
{
	return LoadObject<UStaticMesh>(nullptr, ArthurSwordMeshPath);
}
