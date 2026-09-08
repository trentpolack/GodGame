// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Core/GodGameTypes.h"

#include "CharacterNeedsComponent.generated.h"

class UVillageResourceComponent;

// Runtime state for one character's Need State (range [0.0, 1.0] where 0.0 is satisfied and 1.0 is critical).
USTRUCT(BlueprintType)
struct GODGAME_API FGodGameNeedState
{
	GENERATED_BODY()

	/**
	 * Constructor.
	 */
	FGodGameNeedState() = default;

	/**
	 * Constructor for in-line initialization.
	 * @param NeedTagIn The need represented by this state.
	 * @param ValueIn The initial urgency value.
	 * @param DecayPerSecondIn The urgency gained per second before global scaling.
	 */
	FGodGameNeedState(const FGameplayTag& NeedTagIn, float ValueIn, float DecayPerSecondIn)
	: Need(NeedTagIn), Value(ValueIn), DecayPerSecond(DecayPerSecondIn)
	{
	}

	// Need represented by this state.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need", meta = (GameplayTagFilter="GodGame.Need"))
	FGameplayTag Need = FGameplayTag::EmptyTag;

	// Current urgency in the inclusive range [0, 1].
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Value = 1.0f;

	// Urgency gained per second before the component's decay multiplier is applied.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float DecayPerSecond = 0.01f;
};

/** Broadcast when a villager need's urgency changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNeedChanged, const FGameplayTag&, NeedTag, float, ValueNew, float, NeedModifier);

// Coarse needs model intended to feed StateTree/Blueprint decisions, not replace a full AI system.
UCLASS(ClassGroup=(GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UCharacterNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	// Hysteresis state, updated before notifying external listeners.
	UPROPERTY(Transient)
	FGameplayTagContainer CriticalNeeds;

	// Hysteresis state, updated before notifying external listeners.
	//	TODO (trent, 9/8/26): Need to account for this state properly; it's likely stuck `true` unless editor content is tracking it.
	UPROPERTY(Transient)
	uint8 bConsumingFood : 1 = false;

	/**
	 * Emits a tagged JoyCore payload for a need change event.
	 * @param EventTag The event tag to emit.
	 * @param NeedTag The need tag to emit.
	 * @param ValuePrevious The previous urgency value.
	 * @param ValueNew The new urgency value.
	 */
	UFUNCTION()
	virtual void EmitNeedEvent(const FGameplayTag& EventTag, const FGameplayTag& NeedTag, float ValuePrevious, float ValueNew);

	/**
	 * Finds a mutable need state by Gameplay Tag.
	 * @param NeedTag The need to locate.
	 * @return The matching state, or null when no state matches.
	 */
	FGodGameNeedState* FindNeedMutable(const FGameplayTag& NeedTag);

	/**
	 * Finds a read-only need state by Gameplay Tag.
	 * @param NeedTag The need to locate.
	 * @return The matching state, or null when no state matches.
	 */
	const FGodGameNeedState* FindNeed(const FGameplayTag& NeedTag) const;

protected:
	// Per-need urgency and decay state for the component owner; ::BaseNeeds and ::AdditionalNeeds are added onto this list at startup.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|Needs")
	TArray<FGodGameNeedState> Needs;

	// Default set of values for the base Needs types.
	static TArray<FGodGameNeedState> kBaseNeeds;

public:
	/**
	 * Constructor.
	 */
	UCharacterNeedsComponent();

	// Need enters the critical state at this urgency and recovers at RecoveryThreshold.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float CriticalThreshold = 0.8f;

	// Lower recovery threshold prevents repeated alerts near the critical threshold.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float RecoveryThreshold = 0.6f;
	
	// Assign a settlement inventory to automatically feed this character when hungry.
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Config|Needs|Food")
	TObjectPtr<UVillageResourceComponent> FoodSource = nullptr;

	// Per-need urgency and decay state for the component owner; if none are specified, a default set of Needs and properties are used.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config|Needs")
	TArray<FGodGameNeedState> AdditionalNeeds;

	// Whether configured needs gain urgency during component ticks.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs")
	uint8 bDecayNeeds : 1 = true;

	// Global multiplier applied to every need's DecayPerSecond.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (EditCondition="bDecayNeeds", EditConditionHides, ClampMin="0.0", UIMin="0.0"))
	float DecayMultiplier = 1.0f;
	
	// Hunger required before automatically consuming a meal.
	// TODO (trent, 9/8/26): Data-drive this and integrate it into the Villager AI.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs|Hunger", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float EatAtHunger = 0.6f;

	// Food units spent per meal.
	// TODO (trent, 9/8/26): Data-drive this and integrate it into the Villager AI.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs|Hunger", meta = (ClampMin="0.01", UIMin="0.01"))
	float FoodPerMeal = 1.0f;

	// Urgency removed by one meal.
	// TODO (trent, 9/8/26): Data-drive this and integrate it into the Villager AI.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs|Hunger", meta = (ClampMin="0.01", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float HungerReliefPerMeal = 0.4f;

	// Requested interval in seconds between need-decay updates.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (ClampMin="0.01", UIMin="0.01"))
	float UpdateInterval = 0.25f;

	// Draw debug information.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs")
	uint8 bDebugDraw : 1 = false;

	// Debug draw text offset.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (EditConditionHides, EditCondition="bDebugDraw"))
	FVector DebugDrawBaseOffset = FVector(0.0f, 0.0f, 250.0f);

	// Debug draw text offset per line.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (EditConditionHides, EditCondition="bDebugDraw"))
	FVector DebugDrawLineOffset = FVector(0.0f, 0.0f, -25.0f);

	// Debug draw text scale.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config|Needs", meta = (EditConditionHides, EditCondition="bDebugDraw"))
	float DebugDrawScale = 1.0f;

	// Event raised whenever an operation applies a nonzero urgency delta.
	//	TODO (trent, 9/8/26): Remove this when the JoyCore integration is solid.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Needs")
	FOnNeedChanged OnNeedChanged;

	/**
	 * Consumes one meal from an explicit inventory; fails without spending food when already satisfied.
	 * TODO (trent, 9/8/26): Data-drive this and integrate it into the Villager AI.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs")
	virtual bool TryEat(UVillageResourceComponent* Resources);

	/** Returns whether a need is currently critical, including recovery hysteresis. */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs")
	bool IsNeedCritical(const FGameplayTag& NeedTag) const;

	/** Get the character's overall wellbeing ([0.0, 1.0] critical to satisfied). */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs")
	virtual float GetWellbeing() const;

	/**
	 * Adds a signed delta to a need, creating its state when absent and clamping to [0.0, 1.0].
	 * @param NeedTag The need to modify.
	 * @param Delta The amount to add to urgency.
	 * @return The resulting urgency value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs", meta = (GameplayTagFilter="GodGame.Need"))
	float ModifyNeed(const FGameplayTag& NeedTag, float Delta);

	/**
	 * Replaces a need's value, creating its state when absent and clamping to [0.0, 1.0].
	 * @param NeedTag The need to set.
	 * @param ValueNew The desired urgency value.
	 * @return The resulting clamped urgency value.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Needs", meta = (GameplayTagFilter="GodGame.Need"))
	float SetNeedValue(const FGameplayTag& NeedTag, float ValueNew);

	/**
	 * Retrieves a need's current urgency.
	 * @param NeedTag The need to query.
	 * @return Urgency in [0.0, 1.0], or zero when the need has no state.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs", meta = (GameplayTagFilter="GodGame.Need"))
	float GetNeedValue(const FGameplayTag& NeedTag) const;

	/**
	 * Converts urgency to an urgency score suitable for StateTree or utility decisions.
	 * @param NeedTag The need to query.
	 * @return Urgency where zero is completely satisfied and one is critical.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs", meta = (GameplayTagFilter="GodGame.Need"))
	float GetNeedUrgency(const FGameplayTag& NeedTag) const;

	/**
	 * Finds the configured need with the highest urgency value.
	 * @return The most urgent need, or an empty tag when Needs is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Needs", meta = (GameplayTagFilter="GodGame.Need"))
	FGameplayTag GetMostUrgentNeed() const;

	/**
	 * Replaces Needs with tuned prototype states for Hunger, Rest, Fear, Faith, and Socialization.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "GodGame|Needs")
	virtual void ResetToDefaults();

	// UActorComponent.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~UActorComponent.
};
