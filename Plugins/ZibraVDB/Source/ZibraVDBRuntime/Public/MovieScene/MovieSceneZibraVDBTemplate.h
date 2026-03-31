// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Evaluation/MovieSceneEvalTemplate.h"
#include "MovieScene/MovieSceneZibraVDBSection.h"
#include "MovieSceneZibraVDBTemplate.generated.h"

class UZibraVDBPlaybackComponent;
class UZibraVDBVolume4;

USTRUCT()
struct FMovieSceneZibraVDBSectionTemplateParameters : public FMovieSceneZibraVDBParams
{
	GENERATED_BODY()

	FMovieSceneZibraVDBSectionTemplateParameters() {}
	FMovieSceneZibraVDBSectionTemplateParameters(const FMovieSceneZibraVDBParams& BaseParams, FFrameNumber InSectionStartTime, FFrameNumber InSectionEndTime)
		: FMovieSceneZibraVDBParams(BaseParams)
		, SectionStartTime(InSectionStartTime)
		, SectionEndTime(InSectionEndTime)
	{}

	float MapTimeToAnimation(FFrameTime InPosition, FFrameRate InFrameRate, const UZibraVDBPlaybackComponent* PlaybackComponent, const UZibraVDBVolume4* VDBAsset) const;

	UPROPERTY()
	FFrameNumber SectionStartTime;

	UPROPERTY()
	FFrameNumber SectionEndTime;
};

USTRUCT()
struct FMovieSceneZibraVDBSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

	FMovieSceneZibraVDBSectionTemplate() {}
	FMovieSceneZibraVDBSectionTemplate(const UMovieSceneZibraVDBSection& Section);

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
	FMovieSceneZibraVDBSectionTemplateParameters Params;
};
