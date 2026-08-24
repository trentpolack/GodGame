// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Core/GodGameTypes.h"

#include "VillageResourceComponent.generated.h"

/** Broadcast when a village resource amount changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVillageResourceChanged, FGameplayTag, ResourceTag, float, NewAmount, float, Delta);

// Rough settlement inventory. Put one on a BP_Village rather than simulating individual villager inventories.
UCLASS(ClassGroup = (GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UVillageResourceComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	// TODO (trent, 8/24/26): Rework these accessors.
	/**
	 * Finds a mutable resource state by exact tag.
	 * @param ResourceTag The exact gameplay tag to locate.
	 * @return The matching state, or null when no resource matches.
	 */
	FGodGameResourceState* FindResource(FGameplayTag ResourceTag);

	/**
	 * Finds a read-only resource state by exact tag.
	 * @param ResourceTag The exact gameplay tag to locate.
	 * @return The matching state, or null when no resource matches.
	 */
	const FGodGameResourceState* FindResource(FGameplayTag ResourceTag) const;

public:
	/**
	 * Constructor.
	 */
	UVillageResourceComponent();
	
	// Resource states stored by this settlement.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Resources")
	TArray<FGodGameResourceState> Resources;

	// Whether component ticks apply each resource's PassiveDeltaPerSecond.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Resources")
	uint8 bApplyPassiveResourceDeltas : 1 = true;

	// Requested interval in seconds between passive resource updates.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GodGame|Resources", meta = (ClampMin="0.01", UIMin="0.01"))
	float UpdateInterval = 1.0f;

	// Event raised whenever an operation applies a nonzero resource delta.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Resources")
	FOnVillageResourceChanged OnResourceChanged;

	/**
	 * Adds a signed amount to a resource, creating its state when absent and clamping to capacity.
	 * @param ResourceTag The exact gameplay tag identifying the resource.
	 * @param Amount The amount to add; negative values consume the resource.
	 * @return The resulting amount stored for the resource.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Resources", meta = (GameplayTagFilter="GodGame.Resource"))
	float AddResource(FGameplayTag ResourceTag, float Amount);

	/**
	 * Attempts to consume a nonnegative resource amount atomically.
	 * @param ResourceTag The exact gameplay tag identifying the resource.
	 * @param Amount The nonnegative amount to consume.
	 * @return True when the resource existed and held at least the requested amount.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Resources", meta = (GameplayTagFilter="GodGame.Resource"))
	bool TryConsumeResource(FGameplayTag ResourceTag, float Amount);

	/**
	 * Replaces a resource amount, creating its state when absent and clamping to capacity.
	 * @param ResourceTag The exact gameplay tag identifying the resource.
	 * @param AmountNew The desired stored amount.
	 * @return The resulting clamped resource amount.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Resources", meta = (GameplayTagFilter="GodGame.Resource"))
	float SetResourceAmount(FGameplayTag ResourceTag, float AmountNew);

	/**
	 * Retrieves the current amount of a resource.
	 * @param ResourceTag The exact gameplay tag identifying the resource.
	 * @return The stored amount, or zero when the resource is absent.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Resources", meta = (GameplayTagFilter="GodGame.Resource"))
	float GetResourceAmount(FGameplayTag ResourceTag) const;

	/**
	 * Retrieves resource storage as a fraction of capacity.
	 * @param ResourceTag The exact gameplay tag identifying the resource.
	 * @return Amount divided by capacity, or zero when the resource is absent or has no capacity.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Resources", meta = (GameplayTagFilter="GodGame.Resource"))
	float GetResourceNormalized(FGameplayTag ResourceTag) const;

	/**
	 * Replaces Resources with Food and Wood entries.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "GodGame|Resources")
	void ResetToDefaults();
	
	// UActorComponent.
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~UActorComponent.
};
