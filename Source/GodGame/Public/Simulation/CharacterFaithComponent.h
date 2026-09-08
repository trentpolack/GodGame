// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "GodGameNativeGameplayTags.h"

#include "CharacterFaithComponent.generated.h"

// Additional logic for the Faith "Need" value provided by `CharacterNeedsComponent` (a requirement) as it's somewhat more complex.
//	NOTE: This component auto-registers with the world subsystem during play.
UCLASS(ClassGroup = (GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UCharacterFaithComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructor.
	 */
	UCharacterFaithComponent();

	// Need representing lack of faith; belief strength is one minus this need's urgency.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith")
	FGameplayTag FaithTag = TAG_GodGame_Need_Faith;

	// Minimum faith at which the owner is considered a believer ([0.0, 1.0]).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float BelieverThreshold = 0.20f;

	// Influence generated per second when Faith is 1.0 and contribution is enabled.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith", meta = (ClampMin="0.0", UIMin="0.0"))
	float InfluenceGenerationPerSecondAtMaxFaith = 0.15f;

	// Whether this believer contributes to the world's faith-based influence generation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith")
	uint8 bContributesInfluence : 1 = true;

	/**
	 * Get the current Faith amount from the owning character.
	 * @return Belief strength in `[0.0, 1.0]`, where `1.0` is completely faithful.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Faith")
	float GetFaith() const;

	/**
	 * Replaces Faith with a clamped value and broadcasts changes.
	 * @param FaithNew The desired faith value.
	 * @return The resulting value in the range `[0.0, 1.0]`.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Faith")
	float SetFaith(float FaithNew);

	/**
	 * Adds a signed delta to Faith and clamps the result to `[0.0, 1.0]`.
	 * @param Delta The amount to add; positive values strengthen faith and negative values weaken it.
	 * @return The resulting clamped faith value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Faith")
	float ModifyFaith(float Delta);

	/**
	 * Whether this component's owner is a believer.
	 * @return True when Faith is greater than or equal to `BelieverThreshold`.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Faith")
	virtual bool IsBeliever() const;

	/**
	 * Calculates this component's current faith-based influence contribution.
	 * @return Influence generated per second, or zero when the contribution is disabled or the owner is not a believer.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Faith")
	float GetInfluenceGenerationRate() const;

	// UActorComponent.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~UActorComponent.
};
