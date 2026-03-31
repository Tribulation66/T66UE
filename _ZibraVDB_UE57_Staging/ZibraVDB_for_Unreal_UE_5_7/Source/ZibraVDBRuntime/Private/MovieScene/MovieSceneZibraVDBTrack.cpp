// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "MovieScene/MovieSceneZibraVDBTrack.h"
#include "ZibraVDBPlaybackComponent.h"
#include "MovieScene/MovieSceneZibraVDBTemplate.h"
#include "MovieScene/MovieSceneZibraVDBSection.h"
#include "MovieScene.h"
#include "ZibraVDBSequencerUtils.h"
#include "ZibraVDBVolume4.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneZibraVDBTrack)

#define LOCTEXT_NAMESPACE "MovieSceneZibraVDBTrack"

UMovieSceneZibraVDBTrack::UMovieSceneZibraVDBTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(184, 136, 255, 255);
#endif

	SupportedBlendTypes.Add(EMovieSceneBlendType::Absolute);
	EvalOptions.bCanEvaluateNearestSection = true;
	EvalOptions.bEvaluateInPreroll = true;
}

UMovieSceneSection* UMovieSceneZibraVDBTrack::AddNewAnimation(FFrameNumber KeyTime, UZibraVDBPlaybackComponent* PlaybackComp)
{
	UMovieSceneZibraVDBSection* NewSection = Cast<UMovieSceneZibraVDBSection>(CreateNewSection());

	int32 FrameCount = 0;
	float VDBFPS = ZibraVDBSequencerUtils::DefaultFramerate;

	if (PlaybackComp && PlaybackComp->GetOwner())
	{
		VDBFPS = PlaybackComp->Framerate;

		const UZibraVDBVolume4* Volume = ZibraVDBSequencerUtils::GetVDBAsset(PlaybackComp->GetOwner());
		if (Volume)
		{
			FrameCount = Volume->FrameCount;
		}
	}

	if (VDBFPS < 1.0f)
	{
		// We assume that if effect's FPS is very small it was a mistake
		// and that user will reset it back to 30 after creating track
		VDBFPS = ZibraVDBSequencerUtils::DefaultFramerate;
	}

	NewSection->Params.FrameCount = FrameCount;
	NewSection->Params.Framerate = VDBFPS;

	{
		UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();
		FFrameRate TickResolution = MovieScene->GetTickResolution();

		// We intentionally ignore Playback StartFrame
		// By default we create a track that plays back whole sequence, and ends with same frame it started with

		double AnimationLengthSeconds = FrameCount / VDBFPS;

		FFrameTime AnimationLength = TickResolution.AsFrameTime(AnimationLengthSeconds);
		int32 IFrameNumber = AnimationLength.FrameNumber.Value + static_cast<int>(AnimationLength.GetSubFrame() + 0.5f);

		NewSection->InitialPlacementOnRow(AnimationSections, KeyTime, IFrameNumber, INDEX_NONE);
	}

	AddSection(*NewSection);

	return NewSection;
}

FMovieSceneEvalTemplatePtr UMovieSceneZibraVDBTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneZibraVDBSectionTemplate(*CastChecked<const UMovieSceneZibraVDBSection>(&InSection));
}

const TArray<UMovieSceneSection*>& UMovieSceneZibraVDBTrack::GetAllSections() const
{
	return AnimationSections;
}


bool UMovieSceneZibraVDBTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneZibraVDBSection::StaticClass();
}

UMovieSceneSection* UMovieSceneZibraVDBTrack::CreateNewSection()
{
	return NewObject<UMovieSceneZibraVDBSection>(this, NAME_None, RF_Transactional);
}

void UMovieSceneZibraVDBTrack::RemoveAllAnimationData()
{
	AnimationSections.Empty();
}

bool UMovieSceneZibraVDBTrack::HasSection(const UMovieSceneSection& Section) const
{
	return AnimationSections.Contains(&Section);
}

void UMovieSceneZibraVDBTrack::AddSection(UMovieSceneSection& Section)
{
	AnimationSections.Add(&Section);
}

void UMovieSceneZibraVDBTrack::RemoveSection(UMovieSceneSection& Section)
{
	AnimationSections.Remove(&Section);
}

void UMovieSceneZibraVDBTrack::RemoveSectionAt(int32 SectionIndex)
{
	AnimationSections.RemoveAt(SectionIndex);
}

bool UMovieSceneZibraVDBTrack::IsEmpty() const
{
	return AnimationSections.Num() == 0;
}

#if WITH_EDITORONLY_DATA

FText UMovieSceneZibraVDBTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("TrackName", "ZibraVDB Playback");
}

#endif


#undef LOCTEXT_NAMESPACE
