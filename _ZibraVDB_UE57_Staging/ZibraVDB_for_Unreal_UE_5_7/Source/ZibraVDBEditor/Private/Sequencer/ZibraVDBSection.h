// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISequencerSection.h"

class UMovieSceneZibraVDBSection;
class ISequencer;

/**
 * Sequencer UI section class for ZibraVDB
 */
class FZibraVDBSection : public FSequencerSection
{
public:
	FZibraVDBSection(UMovieSceneSection& InSection, FGuid InObjectBinding, TWeakPtr<ISequencer> InSequencer);

	virtual int32 OnPaintSection(FSequencerSectionPainter& Painter) const override;

private:
	TWeakPtr<ISequencer> Sequencer;
	TWeakObjectPtr<UMovieSceneZibraVDBSection> VDBSectionPtr;
	FGuid ObjectBinding;
};
