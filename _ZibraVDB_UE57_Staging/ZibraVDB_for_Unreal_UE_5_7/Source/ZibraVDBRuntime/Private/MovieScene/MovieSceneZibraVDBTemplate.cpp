// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "MovieScene/MovieSceneZibraVDBTemplate.h"
#include "Evaluation/MovieSceneExecutionTokens.h"
#include "MovieScene.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBVolume4.h"
#include "ZibraVDBSequencerUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneZibraVDBTemplate)


DECLARE_CYCLE_STAT(TEXT("ZibraVDB Evaluate"), MovieSceneEval_ZibraVDB_Evaluate, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("ZibraVDB Token Execute"), MovieSceneEval_ZibraVDB_TokenExecute, STATGROUP_MovieSceneEval);

/** Used to set visibility back to previous when outside section */
struct FPreAnimatedZibraVDBTokenProducer : IMovieScenePreAnimatedTokenProducer
{
	virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const
	{
		struct FToken : IMovieScenePreAnimatedToken
		{
			FToken(UZibraVDBPlaybackComponent* InComponent)
			{
				bWasAnimating = InComponent->Animate;
			}

			virtual void RestoreState(UObject& Object, const UE::MovieScene::FRestoreStateParams& Params)
			{
				UZibraVDBPlaybackComponent* Component = CastChecked<UZibraVDBPlaybackComponent>(&Object);
				Component->Animate = bWasAnimating;
				const bool bHideWhenOutside = Component->SequencerShouldHideWhenOutside;
				Component->Visible = !bHideWhenOutside;
			}

			bool bWasAnimating;
		};

		return FToken(CastChecked<UZibraVDBPlaybackComponent>(&Object));
	}

	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FPreAnimatedZibraVDBTokenProducer>();
	}
};

struct FZibraVDBExecutionToken : IMovieSceneExecutionToken
{
	FZibraVDBExecutionToken(const FMovieSceneZibraVDBSectionTemplateParameters& InParams)
		: Params(InParams)
	{
	}

	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_ZibraVDB_TokenExecute)

		const FFrameTime CurrentTime = Context.GetTime();

		if (Operand.ObjectBindingID.IsValid())
		{
			for (TWeakObjectPtr<> WeakObj : Player.FindBoundObjects(Operand))
			{
				if (UObject* Obj = WeakObj.Get())
				{
					UZibraVDBPlaybackComponent* PlaybackComp = ZibraVDBSequencerUtils::GetPlaybackComponent(Obj);
					const UZibraVDBVolume4* VDBAsset = ZibraVDBSequencerUtils::GetVDBAsset(Obj);
					if (PlaybackComp && PlaybackComp->IsRegistered())
					{
						Player.SavePreAnimatedState(*PlaybackComp, FPreAnimatedZibraVDBTokenProducer::GetAnimTypeID(),
							FPreAnimatedZibraVDBTokenProducer());

						PlaybackComp->SequencerShouldHideWhenOutside = Params.bHideWhenOutside;

						PlaybackComp->Animate = false;
						PlaybackComp->Visible = true;
						float EvalFrame = Params.MapTimeToAnimation(CurrentTime, Context.GetFrameRate(), PlaybackComp, VDBAsset);
						PlaybackComp->CurrentFrame = EvalFrame;
					}
				}
			}
		}
	}

	FMovieSceneZibraVDBSectionTemplateParameters Params;
};

FMovieSceneZibraVDBSectionTemplate::FMovieSceneZibraVDBSectionTemplate(const UMovieSceneZibraVDBSection& InSection)
	: Params(const_cast<FMovieSceneZibraVDBParams&> (InSection.Params), InSection.GetInclusiveStartFrame(), InSection.GetExclusiveEndFrame())
{
}

void FMovieSceneZibraVDBSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_ZibraVDB_Evaluate)
	ExecutionTokens.Add(FZibraVDBExecutionToken(Params));
}

float FMovieSceneZibraVDBSectionTemplateParameters::MapTimeToAnimation(FFrameTime InPosition, FFrameRate InFrameRate, const UZibraVDBPlaybackComponent* PlaybackComponent, const UZibraVDBVolume4* VDBAsset) const
{
	if (!VDBAsset || !PlaybackComponent)
	{
		return 0.0f;
	}

	FFrameTime OffsetFromStart = InPosition - SectionStartTime;
	float VDBFrame = PlaybackComponent->StartFrame + InFrameRate.AsSeconds(OffsetFromStart) * PlaybackComponent->Framerate;
	VDBFrame = FMath::Fmod(VDBFrame, VDBAsset->FrameCount);

	return VDBFrame;
}
