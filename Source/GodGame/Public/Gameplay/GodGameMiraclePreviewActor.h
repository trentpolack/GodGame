// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "Core/GodGameTypes.h"

#include "GodGameMiraclePreviewActor.generated.h"

// Declarations.
class UGodGameMiracleDefinition;
class USceneComponent;

// Lightweight base class for content-authored miracle previews.
UCLASS(Blueprintable, ClassGroup=(GodGame))
class GODGAME_API AGodGameMiraclePreviewActor : public AActor
{
	GENERATED_BODY()

protected:
	/**
	 * Notifies Blueprint presentation that preview placement or validity changed.
	 * @param Miracle The miracle being previewed.
	 * @param Hit The blocking cursor hit used for placement.
	 * @param bCanCast Whether the miracle can currently be cast at the hit.
	 * @param FailureReason The reason casting is unavailable, or None when valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GodGame|MiraclePreview", meta = (DisplayName="Preview Updated"))
	void OnPreviewUpdated(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit, bool bCanCast, EGodGameMiracleCastFailure FailureReason);

public:
	// Root used by Blueprint-authored decals, meshes, and Niagara components.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GodGame|MiraclePreview")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;
	
	/** 
	 * Constructor.
	 */
	AGodGameMiraclePreviewActor();

	/**
	 * Places and displays the preview for a prospective cast.
	 * @param Miracle The miracle being previewed; a null value hides the actor.
	 * @param Hit The cursor trace result used to position and orient the preview.
	 * @param CastCheck The current cast validation result forwarded to Blueprint presentation.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|MiraclePreview")
	void UpdatePreview(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit, const FGodGameMiracleCastCheck& CastCheck);
};
