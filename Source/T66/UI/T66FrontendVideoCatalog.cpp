// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendVideoCatalog.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	static const TCHAR* FrontendVideoManifestPath = TEXT("RuntimeDependencies/T66/Video/frontend_videos.json");

	FString NormalizeKey(const FString& Value)
	{
		FString Key = Value.TrimStartAndEnd();
		Key.ToLowerInline();
		return Key;
	}

	FString BodyTypeToCatalogKey(const ET66BodyType BodyType)
	{
		return BodyType == ET66BodyType::Stacy ? TEXT("stacy") : TEXT("chad");
	}

	TSharedPtr<FJsonObject> LoadFrontendVideoManifest()
	{
		const FString AbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / FrontendVideoManifestPath);
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *AbsolutePath))
		{
			return nullptr;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return nullptr;
		}

		return Root;
	}

	bool ReadVideoAsset(const TSharedPtr<FJsonObject>& Object, FT66FrontendVideoAsset& OutAsset)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		FString MoviePath;
		OutAsset.bPosterOnly = false;
		Object->TryGetStringField(TEXT("movie"), MoviePath);
		Object->TryGetStringField(TEXT("poster"), OutAsset.PosterPath);
		Object->TryGetBoolField(TEXT("posterOnly"), OutAsset.bPosterOnly);
		if (MoviePath.TrimStartAndEnd().IsEmpty() && !OutAsset.bPosterOnly)
		{
			return false;
		}

		OutAsset.MoviePath = MoviePath.TrimStartAndEnd();
		OutAsset.PosterPath = OutAsset.PosterPath.TrimStartAndEnd();
		if (OutAsset.bPosterOnly && OutAsset.PosterPath.IsEmpty())
		{
			return false;
		}
		return true;
	}

	bool TryGetObjectFieldByNormalizedKey(
		const TSharedPtr<FJsonObject>& Object,
		const FString& DesiredKey,
		TSharedPtr<FJsonObject>& OutObject)
	{
		OutObject.Reset();
		if (!Object.IsValid())
		{
			return false;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
		{
			if (NormalizeKey(Pair.Key) != DesiredKey)
			{
				continue;
			}

			const TSharedPtr<FJsonObject> Candidate = Pair.Value.IsValid() ? Pair.Value->AsObject() : nullptr;
			if (Candidate.IsValid())
			{
				OutObject = Candidate;
				return true;
			}
		}

		return false;
	}

	bool TryReadNestedVideoAsset(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& NormalizedKeys,
		FT66FrontendVideoAsset& OutAsset)
	{
		TSharedPtr<FJsonObject> Cursor = Object;
		for (const FString& Key : NormalizedKeys)
		{
			TSharedPtr<FJsonObject> NextObject;
			if (!TryGetObjectFieldByNormalizedKey(Cursor, Key, NextObject) || !NextObject.IsValid())
			{
				return false;
			}

			Cursor = NextObject;
		}

		return ReadVideoAsset(Cursor, OutAsset);
	}

	bool TryReadFallbackAsset(
		const TSharedPtr<FJsonObject>& HeroSelectionObject,
		const FString& NormalizedFallbackKey,
		FT66FrontendVideoAsset& OutAsset)
	{
		const TSharedPtr<FJsonObject>* FallbacksObject = nullptr;
		if (!HeroSelectionObject.IsValid()
			|| !HeroSelectionObject->TryGetObjectField(TEXT("fallbacks"), FallbacksObject)
			|| !FallbacksObject
			|| !FallbacksObject->IsValid())
		{
			return false;
		}

		TSharedPtr<FJsonObject> FallbackObject;
		return TryGetObjectFieldByNormalizedKey(*FallbacksObject, NormalizedFallbackKey, FallbackObject)
			&& FallbackObject.IsValid()
			&& ReadVideoAsset(FallbackObject, OutAsset);
	}

	bool TryResolveHeroFromArray(
		const TSharedPtr<FJsonObject>& HeroSelectionObject,
		const FString& HeroKey,
		const FString& SkinKey,
		const FString& BodyKey,
		FT66FrontendVideoAsset& OutAsset)
	{
		const TArray<TSharedPtr<FJsonValue>>* Heroes = nullptr;
		if (!HeroSelectionObject.IsValid() || !HeroSelectionObject->TryGetArrayField(TEXT("heroes"), Heroes) || !Heroes)
		{
			return false;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Heroes)
		{
			const TSharedPtr<FJsonObject> Entry = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!Entry.IsValid())
			{
				continue;
			}

			FString EntryHero;
			FString EntrySkin;
			FString EntryBody;
			if (!Entry->TryGetStringField(TEXT("heroId"), EntryHero)
				|| !Entry->TryGetStringField(TEXT("skinId"), EntrySkin)
				|| !Entry->TryGetStringField(TEXT("bodyType"), EntryBody))
			{
				continue;
			}

			if (NormalizeKey(EntryHero) == HeroKey
				&& NormalizeKey(EntrySkin) == SkinKey
				&& NormalizeKey(EntryBody) == BodyKey
				&& ReadVideoAsset(Entry, OutAsset))
			{
				return true;
			}
		}

		return false;
	}

	bool TryResolveCompanionFromArray(
		const TSharedPtr<FJsonObject>& HeroSelectionObject,
		const FString& CompanionKey,
		const FString& SkinKey,
		FT66FrontendVideoAsset& OutAsset)
	{
		const TArray<TSharedPtr<FJsonValue>>* Companions = nullptr;
		if (!HeroSelectionObject.IsValid()
			|| !HeroSelectionObject->TryGetArrayField(TEXT("companions"), Companions)
			|| !Companions)
		{
			return false;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Companions)
		{
			const TSharedPtr<FJsonObject> Entry = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!Entry.IsValid())
			{
				continue;
			}

			FString EntryCompanion;
			FString EntrySkin;
			if (!Entry->TryGetStringField(TEXT("companionId"), EntryCompanion)
				|| !Entry->TryGetStringField(TEXT("skinId"), EntrySkin))
			{
				continue;
			}

			if (NormalizeKey(EntryCompanion) == CompanionKey
				&& NormalizeKey(EntrySkin) == SkinKey
				&& ReadVideoAsset(Entry, OutAsset))
			{
				return true;
			}
		}

		return false;
	}
}

namespace T66FrontendVideoCatalog
{
	bool ResolveMainMenuBackground(FT66FrontendVideoAsset& OutAsset)
	{
		if (const TSharedPtr<FJsonObject> Root = LoadFrontendVideoManifest())
		{
			const TSharedPtr<FJsonObject>* MainMenuObject = nullptr;
			const TSharedPtr<FJsonObject>* BackgroundObject = nullptr;
			if (Root->TryGetObjectField(TEXT("mainMenu"), MainMenuObject)
				&& MainMenuObject
				&& MainMenuObject->IsValid()
				&& (*MainMenuObject)->TryGetObjectField(TEXT("background"), BackgroundObject)
				&& BackgroundObject
				&& BackgroundObject->IsValid()
				&& ReadVideoAsset(*BackgroundObject, OutAsset))
			{
				return true;
			}
		}

		OutAsset.MoviePath = TEXT("MainMenuBackground.mp4");
		OutAsset.PosterPath = TEXT("RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/ScreenArt/mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_clean_bloodyretro_1920.png");
		return true;
	}

	bool ResolveHeroSelection(const FName HeroID, const FName SkinID, const ET66BodyType BodyType, FT66FrontendVideoAsset& OutAsset)
	{
		const FString HeroKey = NormalizeKey(HeroID.ToString());
		const FString SkinKey = NormalizeKey(SkinID.IsNone() ? TEXT("Default") : SkinID.ToString());
		const FString BodyKey = BodyTypeToCatalogKey(BodyType);

		if (const TSharedPtr<FJsonObject> Root = LoadFrontendVideoManifest())
		{
			const TSharedPtr<FJsonObject>* HeroSelectionObject = nullptr;
			if (Root->TryGetObjectField(TEXT("heroSelection"), HeroSelectionObject)
				&& HeroSelectionObject
				&& HeroSelectionObject->IsValid())
			{
				const TSharedPtr<FJsonObject>* HeroesObject = nullptr;
				if ((*HeroSelectionObject)->TryGetObjectField(TEXT("heroes"), HeroesObject)
					&& HeroesObject
					&& HeroesObject->IsValid())
				{
					if (TryReadNestedVideoAsset(*HeroesObject, { HeroKey, SkinKey, BodyKey }, OutAsset)
						|| TryReadNestedVideoAsset(*HeroesObject, { HeroKey, NormalizeKey(TEXT("Default")), BodyKey }, OutAsset))
					{
						return true;
					}
				}

				if (TryResolveHeroFromArray(*HeroSelectionObject, HeroKey, SkinKey, BodyKey, OutAsset)
					|| TryResolveHeroFromArray(*HeroSelectionObject, HeroKey, NormalizeKey(TEXT("Default")), BodyKey, OutAsset)
					|| TryReadFallbackAsset(*HeroSelectionObject, BodyType == ET66BodyType::Stacy ? TEXT("herostacy") : TEXT("herochad"), OutAsset)
					|| TryReadFallbackAsset(*HeroSelectionObject, TEXT("hero"), OutAsset))
				{
					return true;
				}

				FString FallbackMovie;
				if ((*HeroSelectionObject)->TryGetStringField(TEXT("fallbackMovie"), FallbackMovie) && !FallbackMovie.TrimStartAndEnd().IsEmpty())
				{
					OutAsset.MoviePath = FallbackMovie.TrimStartAndEnd();
					OutAsset.PosterPath = TEXT("RuntimeDependencies/T66/UI/HeroSelection/Skins/skin_default_stub.png");
					return true;
				}
			}
		}

		OutAsset.MoviePath = TEXT("HeroSelection/Hero_1_Default_Chad.mp4");
		OutAsset.PosterPath = TEXT("RuntimeDependencies/T66/UI/HeroSelection/Skins/skin_default_stub.png");
		return true;
	}

	bool ResolveCompanionSelection(const FName CompanionID, const FName SkinID, FT66FrontendVideoAsset& OutAsset)
	{
		const FString CompanionKey = NormalizeKey(CompanionID.ToString());
		const FString SkinKey = NormalizeKey(SkinID.IsNone() ? TEXT("Default") : SkinID.ToString());

		if (const TSharedPtr<FJsonObject> Root = LoadFrontendVideoManifest())
		{
			const TSharedPtr<FJsonObject>* HeroSelectionObject = nullptr;
			if (Root->TryGetObjectField(TEXT("heroSelection"), HeroSelectionObject)
				&& HeroSelectionObject
				&& HeroSelectionObject->IsValid())
			{
				const TSharedPtr<FJsonObject>* CompanionsObject = nullptr;
				if ((*HeroSelectionObject)->TryGetObjectField(TEXT("companions"), CompanionsObject)
					&& CompanionsObject
					&& CompanionsObject->IsValid())
				{
					if (TryReadNestedVideoAsset(*CompanionsObject, { CompanionKey, SkinKey }, OutAsset)
						|| TryReadNestedVideoAsset(*CompanionsObject, { CompanionKey, NormalizeKey(TEXT("Default")) }, OutAsset))
					{
						return true;
					}
				}

				if (TryResolveCompanionFromArray(*HeroSelectionObject, CompanionKey, SkinKey, OutAsset)
					|| TryResolveCompanionFromArray(*HeroSelectionObject, CompanionKey, NormalizeKey(TEXT("Default")), OutAsset)
					|| TryReadFallbackAsset(*HeroSelectionObject, TEXT("companion"), OutAsset))
				{
					return true;
				}
			}
		}

		OutAsset.MoviePath = TEXT("Frontend/HeroSelection/Companions/Companion_01/Default.mp4");
		OutAsset.PosterPath = TEXT("RuntimeDependencies/T66/Video/Posters/HeroSelection/Companions/Companion_01/Default.png");
		return true;
	}

	FString ResolveMovieAbsolutePath(const FString& MoviePath)
	{
		const FString NormalizedMoviePath = MoviePath.Replace(TEXT("\\"), TEXT("/")).TrimStartAndEnd();
		if (NormalizedMoviePath.IsEmpty())
		{
			return FString();
		}

		const TArray<FString> Candidates = {
			FPaths::ProjectContentDir() / TEXT("Movies") / NormalizedMoviePath,
			FPaths::ProjectDir() / TEXT("Content/Movies") / NormalizedMoviePath
		};

		for (const FString& Candidate : Candidates)
		{
			const FString AbsoluteCandidate = FPaths::ConvertRelativePathToFull(Candidate);
			if (IFileManager::Get().FileExists(*AbsoluteCandidate))
			{
				return AbsoluteCandidate;
			}
		}

		return FPaths::ConvertRelativePathToFull(Candidates[0]);
	}
}
