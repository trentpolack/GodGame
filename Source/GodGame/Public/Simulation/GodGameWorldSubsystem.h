// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Subsystems/WorldSubsystem.h"

#include "GodGameWorldSubsystem.generated.h"

// Declarations.
class UCharacterFaithComponent;

/** Broadcast when the stored influence changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfluenceChanged, float, NewInfluence, float, Delta);

/** Broadcast after registered faith components are aggregated into population metrics. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFaithMetricsChanged, int32, BelieverCount, float, AverageFaith, float, InfluencePerSecond);

/**
 * Coarse global simulation state. It owns influence and aggregates registered FaithComponents,
 * while agent behavior/resources remain local to actors/components.
 */
UCLASS(ClassGroup=(GodGame))
class GODGAME_API UGodGameWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	// Weak set of faith components participating in world aggregation.
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<UCharacterFaithComponent>> FaithComponents;

	// Unprocessed world time carried between coarse simulation steps.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	float SimulationAccumulator = 0.0f;

	// Duration of each coarse simulation step in seconds.
	UPROPERTY(BlueprintReadOnly, AdvancedDisplay, Category = "Config|GodGame|Gameplay")
	float SimulationInterval = 0.5f;

	// Baseline influence generated each second regardless of population faith.
	UPROPERTY(BlueprintReadOnly, AdvancedDisplay, Category = "Config|GodGame|Gameplay")
	float PassiveInfluencePerSecond = 0.0f;

	// Whether FaithInfluencePerSecond is included in influence generation.
	UPROPERTY(BlueprintReadOnly, Category = "Config|GodGame|Gameplay")
	uint8 bGenerateInfluenceFromFaith : 1 = true;
	
	/**
	 * Refreshes population metrics and applies passive and faith-based influence generation.
	 * @param DeltaSeconds Duration of the coarse simulation step in seconds.
	 */
	UFUNCTION(Category = "GodGame|Gameplay")
	virtual void RunSimulationStep(float DeltaSeconds);

	/**
	 * Recalculates believer count, average faith, and faith influence generation.
	 */
	UFUNCTION(Category = "GodGame|Gameplay")
	virtual void RefreshFaithMetrics();

public:
	// TODO (trent, 9/8/26): Need to data-drive this.
	// Current spendable influence, clamped to [0, MaxInfluence].
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	float Influence = 0.0f;

	// Maximum influence the subsystem can store.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	float MaxInfluence = 100.0f;

	// Number of registered faith components currently meeting their believer thresholds.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	int32 BelieverCount = 0;

	// Mean faith value across all valid registered faith components.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	float AverageFaith = 0.0f;

	// Total influence generated per second by registered faith components.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "Transient|GodGame|Gameplay")
	float FaithInfluencePerSecond = 0.0f;

	// Event raised whenever stored influence changes.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Events|Gameplay")
	FOnInfluenceChanged OnInfluenceChanged;

	// Event raised after the subsystem refreshes aggregate faith metrics.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Events|Gameplay")
	FOnFaithMetricsChanged OnFaithMetricsChanged;

	/**
	 * Influence accessor.
	 * @return The current spendable influence.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Influence")
	float GetInfluence() const;

	/**
	 * Attempts to deduct influence without allowing a negative balance.
	 * @param Amount The nonnegative amount to spend.
	 * @return True when the amount was valid and sufficient influence was available.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Influence")
	bool TrySpendInfluence(float Amount);

	/**
	 * Adds a signed amount to influence and clamps the result to the configured range.
	 * @param Amount The amount to add; negative values remove influence.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Influence")
	void AddInfluence(float Amount);

	/**
	 * Replaces the influence balance with a clamped value and broadcasts any applied change.
	 * @param InfluenceNew The desired influence balance.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Influence")
	void SetInfluence(float InfluenceNew);

	/** 
	 * Return current influence remapped into the range `[0.0, 1.0]`.
	 * @return Current influence divided by maximum influence, or zero when the maximum is zero.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Influence")
	float GetInfluenceNormalized() const;

	/** 
	 * Faith component accessor.
	 * @return The number of weak faith-component entries currently registered.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Population")
	int32 GetRegisteredFaithComponentCount() const { return FaithComponents.Num(); }

	/**
	 * Adds a valid faith component to population aggregation.
	 * @param FaithComponent The component to register.
	 */
	UFUNCTION()
	void RegisterFaithComponent(UCharacterFaithComponent* FaithComponent);

	/**
	 * Removes a faith component from population aggregation.
	 * @param FaithComponent The component to unregister.
	 */
	UFUNCTION()
	void UnregisterFaithComponent(UCharacterFaithComponent* FaithComponent);

	// UWorldSubsystem.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// ~UWorldSubsystem.
	
	// FTickableGameObject.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// ~FTickableGameObject.
};
