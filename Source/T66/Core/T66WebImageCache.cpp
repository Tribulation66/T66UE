// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66WebImageCache.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"

UTexture2D* UT66WebImageCache::GetCachedImage(const FString& Url) const
{
	const TObjectPtr<UTexture2D>* Found = CachedTextures.Find(Url);
	return Found ? Found->Get() : nullptr;
}

bool UT66WebImageCache::HasCachedImage(const FString& Url) const
{
	return CachedTextures.Contains(Url);
}

void UT66WebImageCache::RequestImage(const FString& Url, TFunction<void(UTexture2D*)> OnReady)
{
	if (Url.IsEmpty())
	{
		if (OnReady) OnReady(nullptr);
		return;
	}

	// Already cached
	if (UTexture2D* Cached = GetCachedImage(Url))
	{
		if (OnReady) OnReady(Cached);
		return;
	}

	// Queue the waiter
	Waiters.FindOrAdd(Url).Add(MoveTemp(OnReady));

	// If already downloading, just wait
	if (PendingDownloads.Contains(Url))
	{
		return;
	}
	PendingDownloads.Add(Url);

	// Start HTTP download
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetTimeout(15.0f);

	FString CapturedUrl = Url;
	Request->OnProcessRequestComplete().BindLambda(
		[this, CapturedUrl](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConnected)
		{
			if (bConnected && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				OnDownloadComplete(CapturedUrl, Resp->GetContent(), true);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WebImageCache: download failed for %s (code=%d)"),
					*CapturedUrl, Resp.IsValid() ? Resp->GetResponseCode() : 0);
				OnDownloadComplete(CapturedUrl, TArray<uint8>(), false);
			}
		});

	Request->ProcessRequest();
}

void UT66WebImageCache::OnDownloadComplete(const FString& Url, const TArray<uint8>& Data, bool bSuccess)
{
	PendingDownloads.Remove(Url);

	UTexture2D* Texture = nullptr;
	if (bSuccess && Data.Num() > 0)
	{
		Texture = CreateTextureFromData(Data, Url);
		if (Texture)
		{
			CachedTextures.Add(Url, Texture);
		}
	}

	// Notify waiters
	if (TArray<TFunction<void(UTexture2D*)>>* WaiterList = Waiters.Find(Url))
	{
		for (auto& Cb : *WaiterList)
		{
			if (Cb) Cb(Texture);
		}
		Waiters.Remove(Url);
	}

	// Broadcast for raw listeners
	OnWebImageReady.Broadcast(Url, Texture);
}

UTexture2D* UT66WebImageCache::CreateTextureFromData(const TArray<uint8>& Data, const FString& Url)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	// Detect format from data
	EImageFormat Format = ImageWrapperModule.DetectImageFormat(Data.GetData(), Data.Num());
	if (Format == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Warning, TEXT("WebImageCache: unknown image format for %s"), *Url);
		return nullptr;
	}

	TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(Data.GetData(), Data.Num()))
	{
		UE_LOG(LogTemp, Warning, TEXT("WebImageCache: failed to decompress image for %s"), *Url);
		return nullptr;
	}

	TArray<uint8> RawData;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		UE_LOG(LogTemp, Warning, TEXT("WebImageCache: failed to get raw data for %s"), *Url);
		return nullptr;
	}

	const int32 Width = Wrapper->GetWidth();
	const int32 Height = Wrapper->GetHeight();

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		return nullptr;
	}

	Texture->AddToRoot(); // Prevent GC until we manage it

	void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}
