// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "Core/GodGameTypes.h"

#include "GodGameMiracleDefinition.generated.h"

// Declarations.
class AGodGameMiraclePreviewActor;
class UTexture2D;

// Content-facing miracle definition. Make one Primary Data Asset per miracle, then point it at Blueprint actor classes for the actual presentation/effect implementation.
UCLASS(BlueprintType, ClassGroup=(GodGame), Category = "Miracle")
class GODGAME_API UGodGameMiracleDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Localized name shown for the miracle in user interfaces.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|UI")
	FText DisplayName = FText();

	// Localized, potentially multiline description of the miracle.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|UI", meta = (MultiLine="true"))
	FText Description = FText();

	// Soft reference to the icon displayed for the miracle.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|UI")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	// Gameplay tag that uniquely identifies the miracle.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Tags", meta = (GameplayTagFilter="GodGame.Miracle"))
	FGameplayTag MiracleTag = FGameplayTag::EmptyTag;

	// Descriptive traits available to content and gameplay systems.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Tags")
	FGameplayTagContainer MiracleTraits = FGameplayTagContainer::EmptyContainer;

	// Influence deducted when the miracle is cast successfully.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Casting", meta = (ClampMin="0.0", UIMin="0.0"))
	float InfluenceCost = 10.0f;

	// Minimum time in seconds between successful casts of this miracle definition.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Casting", meta = (Units="Seconds", ClampMin="0.0", UIMin="0.0"))
	float CooldownSeconds = 0.0f;

	// 0 means unlimited. Range is measured from the possessed pawn when possible.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Casting", meta = (Units="Centimeters", ClampMin="0.0", UIMin="0.0"))
	float MaxCastRange = 0.0f;

	// Determines which cursor hits are valid targets.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Targeting")
	EMiracleTargetingMode TargetingMode = EMiracleTargetingMode::Ground;

	// Informational radius used by previews, UI, and Blueprint radius helpers.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Targeting", meta = (Units="Centimeters",ClampMin="0.0", UIMin="0.0"))
	float EffectRadius = 400.0f;

	// Whether spawned effect and preview actors orient their up axis to the hit surface normal.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Targeting")
	uint8 bAlignToSurfaceNormal : 1 = false;

	// Spawned when the miracle is actually cast. Usually a short-lived Blueprint effect actor.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Actors")
	TSubclassOf<AActor> MiracleActorClass = nullptr;

	// Optional translucent Blueprint preview. Derive from AGodGameMiraclePreviewActor.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Miracle|Actors")
	TSubclassOf<AGodGameMiraclePreviewActor> PreviewActorClass = nullptr;
};
