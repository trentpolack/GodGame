// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Core/GodGameTypes.h"

#include "GodGameMiracleComponent.generated.h"

// Declarations.
class AGodGameMiraclePreviewActor;
class UGodGameMiracleDefinition;

/** Broadcast when the component's selected miracle changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedMiracleChanged, UGodGameMiracleDefinition*, NewMiracle);

/** Broadcast after a miracle effect actor is spawned and its cost and cooldown are committed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiracleCast, UGodGameMiracleDefinition*, Miracle, AActor*, SpawnedEffectActor);

// Owns the player's current miracle selection, cast validation, cooldowns, and optional preview actor.
UCLASS(ClassGroup=(GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UGodGameMiracleComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// World time of the latest successful cast for each miracle definition.
	//	NOTE (trent, 8/24/26): Fix this so it can be a property.
	TMap<const UGodGameMiracleDefinition*, double> LastCastTimes;
	
	/**
	 * Resolves the location used for range checks.
	 * @return The possessed pawn location, owner location, or world origin when neither is available.
	 */
	FVector GetCastOrigin() const;

	/**
	 * Destroys the current preview and creates the preview class configured by SelectedMiracle, if any.
	 */
	void RecreatePreview();

public:
	/** Constructs a non-ticking miracle component. */
	UGodGameMiracleComponent();

	// Miracle currently selected for validation, previewing, and selected-cast requests.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GodGame|Miracle")
	TObjectPtr<UGodGameMiracleDefinition> SelectedMiracle;

	// Transient preview actor spawned from the selected miracle definition.
	UPROPERTY(BlueprintReadOnly, Transient, AdvancedDisplay, Category = "GodGame|Miracle")
	TObjectPtr<AGodGameMiraclePreviewActor> PreviewActor;

	// Event raised after SelectedMiracle changes.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Events|Miracle")
	FOnSelectedMiracleChanged OnSelectedMiracleChanged;

	// Event raised after a miracle is cast successfully.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Events|Miracle")
	FOnMiracleCast OnMiracleCast;

	/**
	 * Selects a miracle and recreates its optional preview actor.
	 * @param Miracle The miracle to select, or null to clear the selection.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	void SelectMiracle(UGodGameMiracleDefinition* Miracle);

	/** Clears the selected miracle and removes its preview actor. */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	void ClearSelectedMiracle();

	/**
	 * Calculates the cooldown time remaining for a miracle.
	 * @param Miracle The miracle definition to query.
	 * @return Remaining cooldown in seconds, or zero when the miracle is ready or invalid.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Miracle")
	float GetCooldownRemaining(const UGodGameMiracleDefinition* Miracle) const;

	/**
	 * Validates a prospective miracle cast without changing gameplay state.
	 * @param Miracle The miracle definition to validate.
	 * @param Hit The cursor trace result that supplies the prospective target.
	 * @return A structured success or failure result suitable for UI and previews.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	FGodGameMiracleCastCheck CanCastMiracle(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit) const;

	/**
	 * Attempts to spend influence and spawn a miracle effect at a cursor hit.
	 * @param Miracle The miracle definition to cast.
	 * @param Hit The cursor trace result that supplies the target location and normal.
	 * @param SpawnedActor Receives the spawned effect actor, or null when the cast fails.
	 * @param ResultOut Receives the final validation or spawn-failure details.
	 * @return True when the effect actor spawned and the cast was committed.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	bool CastMiracleFromHit(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit, AActor*& SpawnedActor, FGodGameMiracleCastCheck& ResultOut);

	/**
	 * Attempts to cast SelectedMiracle at a cursor hit.
	 * @param Hit The cursor trace result that supplies the target location and normal.
	 * @param SpawnedActor Receives the spawned effect actor, or null when the cast fails.
	 * @param OutResult Receives the final validation or spawn-failure details.
	 * @return True when the selected miracle's effect actor spawned successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	bool CastSelectedMiracleFromHit(const FHitResult& Hit, AActor*& SpawnedActor, FGodGameMiracleCastCheck& OutResult);

	/**
	 * Updates the selected miracle's preview while the player is aiming, creating it lazily when needed.
	 * @param Hit The current cursor trace result used for placement and validation feedback.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	void UpdatePreview(const FHitResult& Hit);

	/**
	 * Hides the active preview actor without destroying it.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Miracle")
	void HidePreview();

	// UActorComponent.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~UActorComponent.
};
