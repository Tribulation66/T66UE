// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66SlateTextureHelpers.h"

#include "Core/T66UITexturePoolSubsystem.h"

#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

namespace T66SlateTexture
{
	UTexture2D* GetLoaded(const UT66UITexturePoolSubsystem* Pool, const TSoftObjectPtr<UTexture2D>& Soft)
	{
		return Pool ? Pool->GetLoadedTexture(Soft) : nullptr;
	}

	void BindSharedBrushAsync(
		UT66UITexturePoolSubsystem* Pool,
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		const TSharedPtr<FSlateBrush>& Brush,
		FName RequestKey,
		bool bClearWhileLoading)
	{
		if (!Pool || !Brush.IsValid())
		{
			return;
		}

		if (Soft.IsNull())
		{
			if (bClearWhileLoading)
			{
				Brush->SetResourceObject(nullptr);
			}
			return;
		}

		if (UTexture2D* Tex = Pool->GetLoadedTexture(Soft))
		{
			Brush->SetResourceObject(Tex);
			return;
		}

		if (bClearWhileLoading)
		{
			Brush->SetResourceObject(nullptr);
		}

		const TWeakPtr<FSlateBrush> WeakBrush(Brush);
		Pool->RequestTexture(Soft, Requester, RequestKey, [WeakBrush](UTexture2D* Loaded)
		{
			if (const TSharedPtr<FSlateBrush> Pinned = WeakBrush.Pin())
			{
				Pinned->SetResourceObject(Loaded);
			}
		});
	}

	void BindBrushAsync(
		UT66UITexturePoolSubsystem* Pool,
		const TSoftObjectPtr<UTexture2D>& Soft,
		UObject* Requester,
		FSlateBrush& Brush,
		FName RequestKey,
		bool bClearWhileLoading)
	{
		if (!Pool)
		{
			return;
		}

		if (Soft.IsNull())
		{
			if (bClearWhileLoading)
			{
				Brush.SetResourceObject(nullptr);
			}
			return;
		}

		if (UTexture2D* Tex = Pool->GetLoadedTexture(Soft))
		{
			Brush.SetResourceObject(Tex);
			return;
		}

		if (bClearWhileLoading)
		{
			Brush.SetResourceObject(nullptr);
		}

		// NOTE: The caller owns the brush memory. We only mutate it if the requester is still valid.
		Pool->RequestTexture(Soft, Requester, RequestKey, [&Brush](UTexture2D* Loaded)
		{
			Brush.SetResourceObject(Loaded);
		});
	}
}

