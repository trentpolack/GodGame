// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "FaithComponent.generated.h"

/** Broadcast when faith changes, including the applied delta and resulting believer state. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFaithChanged, float, NewFaith, float, Delta, bool, bIsBeliever);

/** Individual belief state. Components auto-register with the world subsystem during play. */
UCLASS(ClassGroup = (GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UFaithComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructor.
	 */
	UFaithComponent();
	
	// Current belief strength in the inclusive range [0.0, 1.0].
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Faith = 0.25f;

	// Minimum faith at which the owner is considered a believer ([0.0, 1.0]).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float BelieverThreshold = 0.20f;

	// Influence generated per second when Faith is 1.0 and contribution is enabled.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith", meta = (ClampMin="0.0", UIMin="0.0"))
	float InfluenceGenerationPerSecondAtMaxFaith = 0.15f;

	// Whether this believer contributes to the world's faith-based influence generation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Faith")
	bool bContributesInfluence = true;

	// Event raised whenever an operation changes Faith by a nonzero amount.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Faith")
	FOnFaithChanged OnFaithChanged;

	/**
	 * Adds a signed delta to Faith and clamps the result to [0.0, 1.0].
	 * @param Delta The amount to add; positive values strengthen faith and negative values weaken it.
	 * @return The resulting clamped faith value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Faith")
	float ModifyFaith(float Delta);

	/**
	 * Replaces Faith with a clamped value and broadcasts changes.
	 * @param FaithNew The desired faith value.
	 * @return The resulting value in the range [0.0, 1.0].
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Faith")
	float SetFaith(float FaithNew);

	/**
	 * Tests the current faith against BelieverThreshold.
	 * @return True when Faith is greater than or equal to the believer threshold.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Faith")
	bool IsBeliever() const { return Faith >= BelieverThreshold; }

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
