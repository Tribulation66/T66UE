// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "Sequencer/ZibraVDBTrackEditor.h"

#include "MovieScene/MovieSceneZibraVDBSection.h"
#include "Sequencer/ZibraVDBSection.h"
#include "MovieScene/MovieSceneZibraVDBTrack.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBSequencerUtils.h"
#include "ZibraVDBEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

TSharedRef<ISequencerTrackEditor>
FZibraVDBTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FZibraVDBTrackEditor(InSequencer));
}

FZibraVDBTrackEditor::FZibraVDBTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

bool FZibraVDBTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneZibraVDBTrack::StaticClass();
}

TSharedRef<ISequencerSection>
FZibraVDBTrackEditor::MakeSectionInterface(
	UMovieSceneSection& Section,
	UMovieSceneTrack& Track,
	FGuid ObjectBinding)
{
	return MakeShared<FZibraVDBSection>(Section, ObjectBinding, GetSequencer());
}

void FZibraVDBTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings,
	const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf(AZibraVDBActor::StaticClass()))
	{
		TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
		UObject* BoundObject = SequencerPtr.IsValid() ? SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBindings[0]) : nullptr;
		UZibraVDBPlaybackComponent* PlaybackComp = ZibraVDBSequencerUtils::GetPlaybackComponent(BoundObject);

		if (PlaybackComp)
		{
			UMovieSceneTrack* Track = nullptr;

			MenuBuilder.AddMenuEntry(
				FText::FromString("ZibraVDB Playback"),
				FText::FromString("Adds a ZibraVDB Playback track."),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &FZibraVDBTrackEditor::BuildZibraVDBTrack, ObjectBindings, Track)
					)
				);
		}
	}
}

void FZibraVDBTrackEditor::BuildZibraVDBTrack(TArray<FGuid> ObjectBindings, UMovieSceneTrack* Track)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	if (SequencerPtr.IsValid())
	{
		const FScopedTransaction Transaction(FText::FromString("Add ZibraVDB Playback"));

		for (FGuid ObjectBinding : ObjectBindings)
		{
			if (ObjectBinding.IsValid())
			{
				UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding);
				UZibraVDBPlaybackComponent* PlaybackComp = ZibraVDBSequencerUtils::GetPlaybackComponent(Object);

				if (Object && PlaybackComp)
				{
					AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FZibraVDBTrackEditor::AddKeyInternal, Object,
						PlaybackComp, Track));
				}
			}
		}
	}
}

FKeyPropertyResult FZibraVDBTrackEditor::AddKeyInternal(FFrameNumber KeyTime, UObject* Object,
	UZibraVDBPlaybackComponent* PlaybackComp, UMovieSceneTrack* Track)
{
	FKeyPropertyResult KeyPropertyResult;

	FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
	FGuid ObjectHandle = HandleResult.Handle;
	KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;

	if (ObjectHandle.IsValid())
	{
		if (!Track)
		{
			Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle,
				UMovieSceneZibraVDBTrack::StaticClass(), NAME_None);
			KeyPropertyResult.bTrackCreated = true;
		}

		if (ensure(Track))
		{
			Track->Modify();

			UMovieSceneSection* NewSection = Cast<UMovieSceneZibraVDBTrack>(Track)->AddNewAnimation(KeyTime, PlaybackComp);

			KeyPropertyResult.bTrackModified = true;
			KeyPropertyResult.SectionsCreated.Add(NewSection);

			GetSequencer()->EmptySelection();
			GetSequencer()->SelectSection(NewSection);
			GetSequencer()->ThrobSectionSelection();
		}
	}

	return KeyPropertyResult;
}

TSharedPtr<SWidget> FZibraVDBTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track,
	const FBuildEditWidgetParams& Params)
{
	// Hide the "+" button
	return TSharedPtr<SWidget>();
}
