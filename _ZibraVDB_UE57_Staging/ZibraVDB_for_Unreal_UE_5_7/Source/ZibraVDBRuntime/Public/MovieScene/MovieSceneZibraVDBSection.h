// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "MovieSceneSection.h"
#include "MovieSceneZibraVDBSection.generated.h"

USTRUCT(BlueprintType)
struct ZIBRAVDBRUNTIME_API FMovieSceneZibraVDBParams
{
	GENERATED_BODY()

	FMovieSceneZibraVDBParams();

	UPROPERTY()
	int32 FrameCount;

	UPROPERTY()
	float Framerate;

	// Required by UE::MovieScene::GetFirstLoopStartOffsetAtTrimTime
	UPROPERTY()
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ZibraVDB", DisplayName="Hide When Outside")
	uint32 bHideWhenOutside : 1;
};

UCLASS(MinimalAPI)
class UMovieSceneZibraVDBSection : public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation", meta = (ShowOnlyInnerProperties))
	FMovieSceneZibraVDBParams Params;

protected:
	virtual TOptional<TRange<FFrameNumber>> GetAutoSizeRange() const override;
};
