// Copyright Tribulation 66. All Rights Reserved.

#include "T66CodeReferencedAssets.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CookGuard, Log, All);

namespace
{
	// Object paths ("/Package/Path.ObjectName") the C++ code loads by string. ADD NEW
	// ENTRIES HERE whenever you add a LoadObject/FindObject with a literal path — the
	// startup check is what keeps a forgotten cook rule from shipping silently.
	const TCHAR* GT66CodeReferencedAssetPaths[] =
	{
		// BLACK-tier projectile meshes (T66CombatComponent.cpp / T66CombatVFX.cpp)
		TEXT("/Game/Weapons/Projectiles/FriendSlop/SM_WeaponProjectile_Black.SM_WeaponProjectile_Black"),
		TEXT("/Game/Weapons/Projectiles/FriendSlop/SM_IdolProjectile_FireBlack.SM_IdolProjectile_FireBlack"),
		// The one gameplay master material (T66VisualUtil.cpp)
		TEXT("/Game/Materials/M_FriendSlop_FallGuys.M_FriendSlop_FallGuys"),
		// Data tables resolved by string (T66VehicleInteractable / world visual props)
		TEXT("/Game/Data/DT_VehicleInteractables.DT_VehicleInteractables"),
		TEXT("/Game/Data/DT_WorldVisualProps.DT_WorldVisualProps"),
		// TestRoom environment materials (T66GameMode_TestRoom)
		TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Ceiling.MI_TestRoom_Ceiling"),
		TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Floor.MI_TestRoom_Floor"),
		TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Wall.MI_TestRoom_Wall"),
		// Terrain visuals loaded by string (T66MainMapTerrain)
		TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"),
		TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof"),
		TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround"),
		TEXT("/Game/World/Terrain/TowerForest/T_TowerForestGround.T_TowerForestGround"),
		// Baffle tube: floors/walls/ceilings, bounce platforms, doorway arches (T66TowerMapTerrain)
		TEXT("/Game/World/Terrain/TowerDungeon/Baffles/SM_BaffleTube.SM_BaffleTube"),
		// Fall Guys candy slab kit (T66TowerThemeVisuals / T66TowerMapTerrain)
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Floor.MI_FallGuys_Floor"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Wall.MI_FallGuys_Wall"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Ceiling.MI_FallGuys_Ceiling"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Platform.MI_FallGuys_Platform"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Ramp.MI_FallGuys_Ramp"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Mesa.MI_FallGuys_Mesa"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Lift.MI_FallGuys_Lift"),
		TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_Hex.SM_FGShape_Hex"),
		TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_Tri.SM_FGShape_Tri"),
		TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_BevelCube.SM_FGShape_BevelCube"),
		TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_BevelPuck.SM_FGShape_BevelPuck"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Deck.MI_FallGuys_Deck"),
		TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Trim.MI_FallGuys_Trim"),
		// InflatableTraps01 kit loaded by string (T66ObstacleTrap)
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_SweeperArm.SM_Inflatable_SweeperArm"),
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_Hub.SM_Inflatable_Hub"),
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_Bumper.SM_Inflatable_Bumper"),
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_Pad.SM_Inflatable_Pad"),
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_Mallet.SM_Inflatable_Mallet"),
		TEXT("/Game/World/Traps/Inflatable/SM_Inflatable_Tube.SM_Inflatable_Tube"),
		TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_StripesDiag.MI_Inflatable_StripesDiag"),
		TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_BandsHoriz.MI_Inflatable_BandsHoriz"),
		TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_Chevrons.MI_Inflatable_Chevrons"),
		TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_Stars.MI_Inflatable_Stars"),
		TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_Dots.MI_Inflatable_Dots"),
		// UI glow material (T66 UI widgets)
		TEXT("/Game/UI/Materials/M_UI_Glow.M_UI_Glow"),
		// Engine primitives FT66VisualUtil resolves by string (see DefaultGame.ini comment)
		TEXT("/Engine/BasicShapes/Cone.Cone"),
		TEXT("/Engine/BasicShapes/Cube.Cube"),
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),
		TEXT("/Engine/BasicShapes/Plane.Plane"),
		TEXT("/Engine/BasicShapes/Sphere.Sphere"),
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"),
		TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque.DefaultTextMaterialOpaque"),
		TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"),
		TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"),
		TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"),
	};
}

void T66CodeReferencedAssets::VerifyAllResolvable()
{
	int32 MissingCount = 0;
	for (const TCHAR* ObjectPath : GT66CodeReferencedAssetPaths)
	{
		const FString PackagePath = FPackageName::ObjectPathToPackageName(FString(ObjectPath));
		if (!FPackageName::DoesPackageExist(PackagePath))
		{
			++MissingCount;
			UE_LOG(LogT66CookGuard, Error,
				TEXT("[CookGuard] Code-referenced asset MISSING from this build: %s — LoadObject returns null and gameplay silently falls back to placeholder visuals. Add the asset's directory to DirectoriesToAlwaysCook in Config/DefaultGame.ini (see the /Game/Weapons and /Engine/BasicShapes entries), then restage."),
				ObjectPath);
		}
	}
	if (MissingCount == 0)
	{
		UE_LOG(LogT66CookGuard, Display, TEXT("[CookGuard] All %d code-referenced assets resolve in this build."),
			static_cast<int32>(UE_ARRAY_COUNT(GT66CodeReferencedAssetPaths)));
	}
}
