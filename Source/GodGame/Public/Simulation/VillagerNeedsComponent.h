// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Core/GodGameTypes.h"

#include "VillagerNeedsComponent.generated.h"

/** Broadcast when a villager need's satisfaction changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNeedChanged, EVillagerNeed, Need, float, NewValue, float, Delta);

// Coarse needs model intended to feed StateTree/Blueprint decisions, not replace a full AI system.
UCLASS(ClassGroup=(GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UVillagerNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	/**
	 * Finds a mutable need state by enum value.
	 * @param Need The need to locate.
	 * @return The matching state, or null when no state matches.
	 */
	FGodGameNeedState* FindNeed(EVillagerNeed Need);

	/**
	 * Finds a read-only need state by enum value.
	 * @param Need The need to locate.
	 * @return The matching state, or null when no state matches.
	 */
	const FGodGameNeedState* FindNeed(EVillagerNeed Need) const;

public:
	/** 
	 * Constructor.
	 */
	UVillagerNeedsComponent();
	
	// Per-need satisfaction and decay state for the component owner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Needs")
	TArray<FGodGameNeedState> Needs;

	// Whether configured needs lose satisfaction during component ticks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Needs")
	uint8 bDecayNeeds : 1 = true;

	// Global multiplier applied to every need's DecayPerSecond.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Needs", meta = (ClampMin="0.0", UIMin="0.0"))
	float DecayMultiplier = 1.0f;

	// Requested interval in seconds between need-decay updates.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Needs", meta = (ClampMin="0.05", UIMin="0.01"))
	float UpdateInterval = 0.25f;

	// Whether BeginPlay creates prototype states when Needs is empty.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Needs")
	bool bInitializePrototypeDefaultsIfEmpty = true;

	// Event raised whenever an operation applies a nonzero satisfaction delta.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Needs")
	FOnNeedChanged OnNeedChanged;

	/**
	 * Adds a signed delta to a need, creating its state when absent and clamping to [0.0, 1.0].
	 * @param Need The need to modify.
	 * @param Delta The amount to add to satisfaction.
	 * @return The resulting satisfaction value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs")
	float ModifyNeed(EVillagerNeed Need, float Delta);

	/**
	 * Replaces a need's value, creating its state when absent and clamping to [0.0, 1.0].
	 * @param Need The need to set.
	 * @param ValueNew The desired satisfaction value.
	 * @return The resulting clamped satisfaction value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs")
	float SetNeedValue(EVillagerNeed Need, float ValueNew);

	/**
	 * Increases a need's satisfaction by the absolute supplied amount.
	 * @param Need The need to satisfy.
	 * @param Amount The magnitude of satisfaction to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs")
	void SatisfyNeed(EVillagerNeed Need, float Amount) { ModifyNeed(Need, FMath::Abs(Amount)); }

	/**
	 * Retrieves a need's current satisfaction.
	 * @param Need The need to query.
	 * @return Satisfaction in [0.0, 1.0], or zero when the need has no state.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs")
	float GetNeedValue(EVillagerNeed Need) const;

	/**
	 * Converts satisfaction to an urgency score suitable for StateTree or utility decisions.
	 * @param Need The need to query.
	 * @return Urgency where zero is completely satisfied and one is critical.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs")
	float GetNeedUrgency(EVillagerNeed Need) const;

	/**
	 * Finds the configured need with the lowest satisfaction value.
	 * @return The most urgent need, or Hunger when Needs is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs")
	EVillagerNeed GetMostUrgentNeed() const;

	/**
	 * Replaces Needs with tuned prototype states for Hunger, Rest, Safety, and Faith.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "GodGame|Needs")
	void ResetToPrototypeDefaults();

	// UActorComponent.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~UActorComponent.
};
