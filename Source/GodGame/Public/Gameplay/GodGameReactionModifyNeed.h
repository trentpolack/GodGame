// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "Systems/Reactions/SystemicReaction.h"

#include "GodGameReactionModifyNeed.generated.h"

/** Applies need relief or distress to a JoyCore event target from an authored systemic rule. */
UCLASS(BlueprintType, EditInlineNew, Category="Game|Systems", ClassGroup=(GodGame))
class GODGAME_API UGodGameReactionModifyNeed : public USystemicReaction
{
	GENERATED_BODY()

public:
	// Exact need to change on the target character.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Reaction|Config", meta = (GameplayTagFilter="GodGame.Need"))
	FGameplayTag NeedTag = FGameplayTag::EmptyTag;

	// Negative values relieve the need; positive values increase urgency.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Reaction|Config")
	float NeedModifier = -0.1f;

	// Multiply Delta by the triggering event's Value, for intensity-sensitive reactions.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Reaction|Config")
	uint8 bScaleByEventValue : 1 = false;

	/**
	 * Changes a need on the event target actor or component and records the outcome in the JoyCore trace.
	 * @param Event The triggering event.
	 * @param Context The systemic rule execution context.
	 * @param Trace The JoyCore trace.
	 * @return True if the need was modified successfully.
	 */
	virtual bool Execute(const FSystemicEvent& Event, FSystemicRuleContext& Context, FSystemicTrace& Trace) override;
};
