// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"

#include "GodGamePlayerController.generated.h"

/** Broadcast when the controller changes its selected actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGodGameSelectionChanged, AActor*, NewSelection, AActor*, PreviousSelection);

// Minimal top-down interaction controller.
UCLASS(Blueprintable, ClassGroup=(GodGame))
class GODGAME_API AGodGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/**
	 * Constructor.
	 */
	AGodGamePlayerController();

	// Collision channel used by cursor line traces.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GodGame|Cursor")
	TEnumAsByte<ECollisionChannel> CursorTraceChannel = ECC_Visibility;

	// Trace distance used when TraceGodCursor receives a non-positive override.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GodGame|Cursor", meta = (ClampMin="100.0", UIMin="100.0"))
	float DefaultTraceDistance = 100000.0f;

	// Actor currently selected by this controller, or null when no selection is active.
	UPROPERTY(BlueprintReadOnly, Category = "GodGame|Selection")
	TObjectPtr<AActor> SelectedActor;

	// Event raised after SelectedActor changes.
	UPROPERTY(BlueprintAssignable, Category = "GodGame|Selection")
	FOnGodGameSelectionChanged OnSelectionChanged;

	/**
	 * Traces from the mouse cursor into the world using CursorTraceChannel.
	 * @param HitOut Receives the blocking hit when the trace succeeds.
	 * @param TraceDistance Optional trace length; non-positive values use DefaultTraceDistance.
	 * @return True when the trace produced a blocking hit.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Cursor")
	bool TraceGodCursor(FHitResult& HitOut, float TraceDistance = 0.0f) const;

	/**
	 * Selects the actor under the cursor unless its God Game tag component opts out of selection.
	 * @return The newly selected actor, or null when the cursor has no selectable target.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Selection")
	AActor* SelectActorUnderCursor();

	/**
	 * Replaces the selected actor and broadcasts the old and new selections when they differ.
	 * @param SelectionNew The actor to select or null to clear the selection.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Selection")
	void SetSelectedActor(AActor* SelectionNew);

	/**
	 * Clears the current actor selection.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Selection")
	void ClearSelection();
};
