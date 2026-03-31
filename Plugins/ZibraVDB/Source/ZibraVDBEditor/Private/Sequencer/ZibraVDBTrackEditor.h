// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneTrackEditor.h"

struct FBuildEditWidgetParams;

class FZibraVDBTrackEditor : public FMovieSceneTrackEditor
{
public:
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

	FZibraVDBTrackEditor(TSharedRef<ISequencer> InSequencer);

	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(
		UMovieSceneSection& Section,
		UMovieSceneTrack& Track,
		FGuid ObjectBinding) override;

	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings,
		const UClass* ObjectClass) override;
	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track,
		const FBuildEditWidgetParams& Params) override;

private:
	void BuildZibraVDBTrack(TArray<FGuid> ObjectBindings, UMovieSceneTrack* Track);
	FKeyPropertyResult AddKeyInternal(FFrameNumber KeyTime, UObject* Object, class UZibraVDBPlaybackComponent* PlaybackComp,
		UMovieSceneTrack* Track);
};
