// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

struct FT66FrontendVideoAsset
{
	FString MoviePath;
	FString PosterPath;
	bool bPosterOnly = false;
};

namespace T66FrontendVideoCatalog
{
	T66_API bool ResolveMainMenuBackground(FT66FrontendVideoAsset& OutAsset);
	T66_API bool ResolveHeroSelection(FName HeroID, FName SkinID, ET66BodyType BodyType, FT66FrontendVideoAsset& OutAsset);
	T66_API bool ResolveCompanionSelection(FName CompanionID, FName SkinID, FT66FrontendVideoAsset& OutAsset);
	T66_API FString ResolveMovieAbsolutePath(const FString& MoviePath);
}
