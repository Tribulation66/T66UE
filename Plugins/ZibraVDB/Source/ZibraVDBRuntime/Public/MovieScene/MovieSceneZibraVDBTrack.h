// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "MovieSceneNameableTrack.h"
#include "Compilation/IMovieSceneTrackTemplateProducer.h"
#include "MovieSceneZibraVDBTrack.generated.h"

/**
 * Handles animation of ZibraVDB actors
 */
class UZibraVDBPlaybackComponent;

UCLASS(MinimalAPI)
class UMovieSceneZibraVDBTrack
	: public UMovieSceneNameableTrack
	, public IMovieSceneTrackTemplateProducer
{
	GENERATED_UCLASS_BODY()

	virtual UMovieSceneSection* AddNewAnimation(FFrameNumber KeyTime, UZibraVDBPlaybackComponent* PlaybackComp);

	virtual void RemoveAllAnimationData() override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual void RemoveSectionAt(int32 SectionIndex) override;
	virtual bool IsEmpty() const override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual bool SupportsMultipleRows() const override { return false; }
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
#endif

private:

	UPROPERTY()
	TArray<TObjectPtr<UMovieSceneSection>> AnimationSections;

};
