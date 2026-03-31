// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "Sequencer/ZibraVDBSection.h"
#include "ISequencer.h"
#include "SequencerSectionPainter.h"
#include "TimeToPixel.h"
#include "MovieScene/MovieSceneZibraVDBSection.h"
#include "ZibraVDBVolume4.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBSequencerUtils.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "MovieScene.h"

FZibraVDBSection::FZibraVDBSection(UMovieSceneSection& InSection, FGuid InObjectBinding, TWeakPtr<ISequencer> InSequencer)
	: FSequencerSection(InSection)
	, VDBSectionPtr(Cast<UMovieSceneZibraVDBSection>(&InSection))
	, ObjectBinding(InObjectBinding)
	, Sequencer(InSequencer)
{
}

int32 FZibraVDBSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 Layer = Painter.PaintSectionBackground();

	const UMovieSceneZibraVDBSection* VDBSection = VDBSectionPtr.Get();
	if (!VDBSection)
	{
		return Layer;
	}

	UObject* BoundObject = nullptr;
	TSharedPtr<ISequencer> SequencerPin = Sequencer.Pin();
	if (SequencerPin.IsValid() && ObjectBinding.IsValid())
	{
		BoundObject = SequencerPin->FindSpawnedObjectOrTemplate(ObjectBinding);
	}

	const UZibraVDBVolume4* VDBAsset = ZibraVDBSequencerUtils::GetVDBAsset(BoundObject);
	UZibraVDBPlaybackComponent* PlaybackComponent = ZibraVDBSequencerUtils::GetPlaybackComponent(BoundObject);

	if (!VDBAsset)
	{
		return Layer;
	}

	const auto& TimeToPixelConverter = Painter.GetTimeConverter();
	const FGeometry& SectionGeometry = Painter.SectionGeometry;

	// Draw VDB asset name
	FString AssetName = VDBAsset->GetName();
	const FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Bold", 10);

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FVector2D TextSize = FontMeasure->Measure(AssetName, FontInfo);

	const FVector2f TextPosition(6.0f, (SectionGeometry.GetLocalSize().Y - TextSize.Y) * 0.5f);
	const FVector2f TextSizeFloat(TextSize);

	FSlateDrawElement::MakeText(
		Painter.DrawElements,
		++Layer,
		SectionGeometry.ToPaintGeometry(TextSizeFloat, FSlateLayoutTransform(TextPosition)),
		AssetName,
		FontInfo,
		Painter.bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
		FLinearColor::White
		);

	if (VDBAsset->FrameCount < 2)
	{
		return Layer;
	}

	const double VDBFrameCount = VDBAsset->FrameCount;
	const double VDBFPS = PlaybackComponent ? PlaybackComponent->Framerate : ZibraVDBSequencerUtils::DefaultFramerate;

	const FFrameRate TickResolution = TimeToPixelConverter.GetTickResolution();
	const double EndTime = TickResolution.AsSeconds(VDBSection->GetExclusiveEndFrame());

	double CurrentTime = TickResolution.AsSeconds(VDBSection->GetInclusiveStartFrame());
	double CurrentVDBFrame = PlaybackComponent ? PlaybackComponent->StartFrame : 0.0;

	while (true)
	{
		double TimeToTrackEnd = EndTime - CurrentTime;
		double FramesToSequenceEnd = VDBFrameCount - CurrentVDBFrame;
		if (TimeToTrackEnd * VDBFPS < FramesToSequenceEnd)
		{
			break;
		}
		double TimeToSequenceEnd = FramesToSequenceEnd / VDBFPS;
		double NextTickTime = CurrentTime + TimeToSequenceEnd;

		float NextTickPixel = TimeToPixelConverter.SecondsToPixel(NextTickTime);

		TArray<FVector2D> LinePoints;
		LinePoints.Add(FVector2D(NextTickPixel, 0.0f));
		LinePoints.Add(FVector2D(NextTickPixel, SectionGeometry.GetLocalSize().Y));

		FSlateDrawElement::MakeLines(Painter.DrawElements, ++Layer, SectionGeometry.ToPaintGeometry(), LinePoints,
			Painter.bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
			FLinearColor(1.0f, 1.0f, 1.0f, 0.5f), false, 2.0f);

		CurrentTime = NextTickTime;
		CurrentVDBFrame = 0.0f;
	}

	return Layer;
}
