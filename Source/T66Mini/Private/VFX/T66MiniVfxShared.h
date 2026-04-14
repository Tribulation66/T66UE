// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace T66MiniVfx
{
	inline UStaticMesh* LoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	inline UMaterialInterface* LoadUnlitMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}

	inline UTexture* LoadWhiteTexture()
	{
		static TWeakObjectPtr<UTexture> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		}

		return Cached.Get();
	}

	inline UMaterialInstanceDynamic* ApplyTintedMaterial(
		UPrimitiveComponent* PrimitiveComponent,
		UObject* Outer,
		UTexture* Texture,
		const FLinearColor& Tint)
	{
		if (!PrimitiveComponent)
		{
			return nullptr;
		}

		UMaterialInterface* BaseMaterial = LoadUnlitMaterial();
		if (!BaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(PrimitiveComponent->GetMaterial(0));
		if (!Material)
		{
			Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : PrimitiveComponent);
			if (!Material)
			{
				return nullptr;
			}

			PrimitiveComponent->SetMaterial(0, Material);
		}

		UTexture* TextureToUse = Texture ? Texture : LoadWhiteTexture();
		if (TextureToUse)
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), TextureToUse);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), TextureToUse);
		}

		Material->SetVectorParameterValue(TEXT("Color"), Tint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetVectorParameterValue(TEXT("Tint"), Tint);
		return Material;
	}
}
