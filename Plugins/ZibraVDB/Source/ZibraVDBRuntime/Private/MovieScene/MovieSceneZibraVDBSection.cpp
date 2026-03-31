// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "MovieScene/MovieSceneZibraVDBSection.h"
#include "MovieScene.h"
#include "MovieSceneTimeHelpers.h"

FMovieSceneZibraVDBParams::FMovieSceneZibraVDBParams()
	: FrameCount(0)
	, Framerate(30.0f)
	, PlayRate(1.f)
	, bHideWhenOutside(false)
{
}

UMovieSceneZibraVDBSection::UMovieSceneZibraVDBSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BlendType = EMovieSceneBlendType::Absolute;
	EvalOptions.EnableAndSetCompletionMode(EMovieSceneCompletionMode::RestoreState);
}

TOptional<TRange<FFrameNumber> > UMovieSceneZibraVDBSection::GetAutoSizeRange() const
{
	if (Params.FrameCount <= 0)
	{
		return TOptional<TRange<FFrameNumber>>();
	}

	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();
	FFrameRate TickResolution = MovieScene->GetTickResolution();

	const float FrameCount = Params.FrameCount;
	const float VDBFramerate = FMath::Max(Params.Framerate, UE_KINDA_SMALL_NUMBER);

	double AnimationLengthTicks = FrameCount * (TickResolution.AsDecimal() / VDBFramerate);
	FFrameTime AnimationLength = FFrameTime::FromDecimal(AnimationLengthTicks);
	int32 IFrameNumber = AnimationLength.FrameNumber.Value + (int)(AnimationLength.GetSubFrame() + 0.5f) + 1;

	return TRange<FFrameNumber>(GetInclusiveStartFrame(), GetInclusiveStartFrame() + IFrameNumber);
}
