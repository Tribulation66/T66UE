// Copyright Tribulation 66. All Rights Reserved.

#include "T66ProceduralLandscapeEditorTool.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "T66Editor.h"

#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "T66ProceduralLandscape"

namespace T66ProceduralLandscapeEditor
{
	static constexpr int32 SectionSizeQuads = 63;
	static constexpr int32 SectionsPerComponent = 1;
	static constexpr int32 QuadsPerComponent = SectionSizeQuads * SectionsPerComponent;

	bool GenerateProceduralHillsLandscape(UWorld* World, const FT66ProceduralLandscapeParams& Params)
	{
		if (!World)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: World is null"));
			return false;
		}

		int32 SizeX = 0, SizeY = 0;
		FT66ProceduralLandscapeGenerator::GetDimensionsForPreset(Params.SizePreset, SizeX, SizeY);

		TArray<float> Heights;
		if (!FT66ProceduralLandscapeGenerator::GenerateHeightfield(
			Params, SizeX, SizeY, FT66ProceduralLandscapeGenerator::DefaultQuadSizeUU, Heights))
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: GenerateHeightfield failed"));
			return false;
		}

		TArray<uint16> HeightData;
		const float ScaleZ = 100.f;
		FT66ProceduralLandscapeGenerator::FloatsToLandscapeHeightData(Heights, ScaleZ, HeightData);

		if (HeightData.Num() != SizeX * SizeY)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: height data size mismatch"));
			return false;
		}

		// Debug: log height range to verify variation
		uint16 MinH = 65535, MaxH = 0;
		for (uint16 H : HeightData)
		{
			MinH = FMath::Min(MinH, H);
			MaxH = FMath::Max(MaxH, H);
		}
		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: height range %u - %u (flat=32768)"), MinH, MaxH);

		// Remove any existing Landscape(s) so we always create fresh
		TArray<ALandscape*> ExistingLandscapes;
		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			ExistingLandscapes.Add(*It);
		}
		for (ALandscape* L : ExistingLandscapes)
		{
			if (L)
			{
				UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: removing existing Landscape %s"), *L->GetName());
				L->Destroy();
			}
		}

		// Create new Landscape
		const int32 NumComponentsX = (SizeX - 1) / QuadsPerComponent;
		const int32 NumComponentsY = (SizeY - 1) / QuadsPerComponent;
		if (NumComponentsX < 1 || NumComponentsY < 1)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: invalid component count for size %dx%d"), SizeX, SizeY);
			return false;
		}

		TMap<FGuid, TArray<uint16>> HeightMapForImport;
		HeightMapForImport.Add(FGuid(), MoveTemp(HeightData));

		FActorSpawnParameters SpawnParams;
		ALandscape* Landscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!Landscape)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: failed to SpawnActor ALandscape"));
			return false;
		}

		Landscape->SetActorScale3D(FVector(100.f, 100.f, 100.f));

		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayersForImport;
		MaterialLayerDataPerLayersForImport.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
		TArray<FLandscapeLayer> ImportLayers;
		Landscape->Import(
			FGuid::NewGuid(),
			0, 0,
			SizeX - 1, SizeY - 1,
			SectionsPerComponent,
			QuadsPerComponent,
			HeightMapForImport,
			nullptr,
			MaterialLayerDataPerLayersForImport,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>(ImportLayers));

		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: Landscape->Import completed"));

		Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(
			FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1), (uint32)2);

		ULandscapeInfo* NewInfo = Landscape->GetLandscapeInfo();
		if (NewInfo)
		{
			NewInfo->UpdateLayerInfoMap(Landscape);
		}
		Landscape->RegisterAllComponents();

		// Force visual update
		Landscape->PostEditChange();

		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: created new Landscape (seed=%d, %dx%d)"), Params.Seed, SizeX, SizeY);
		return true;
	}
}

#undef LOCTEXT_NAMESPACE
