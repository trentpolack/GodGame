// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "GodGameTypes.generated.h"

//	TODO (trent, 8/24/26): Most of these should be able to be converted to gameplay tags eventually.

// Declarations.
class UGodGameMiracleDefinition;

// Small, intentionally fixed need set used by the prototype villager simulation.
UENUM(BlueprintType)
enum class EVillagerNeed : uint8
{
	// Food and nourishment satisfaction.
	Hunger	UMETA(DisplayName="Hunger"),

	// Sleep and recovery satisfaction.
	Rest	UMETA(DisplayName="Rest"),

	// Protection and security satisfaction.
	Safety	UMETA(DisplayName="Safety"),

	// Spiritual belief and devotion satisfaction.
	Faith	UMETA(DisplayName="Faith")
};

// Determines what kind of cursor hit a miracle considers valid.
UENUM(BlueprintType)
enum class EMiracleTargetingMode : uint8
{
	// Accepts any blocking surface hit.
	Ground			UMETA(DisplayName="Ground/Surface"),

	// Requires the hit to resolve to an actor.
	Actor			UMETA(DisplayName="Actor"),

	// Accepts either a blocking surface or an actor.
	GroundOrActor	UMETA(DisplayName="Ground/Surface or Actor")
};

// Useful for UI, preview tinting, and quick Blueprint debugging.
UENUM(BlueprintType)
enum class EGodGameMiracleCastFailure : uint8
{
	// The cast is valid and has no failure.
	None,

	// No miracle definition was supplied or selected, or the miracle is on cooldown.
	NoMiracleSelectedOrOnCooldown,

	// The cursor hit does not satisfy the miracle's targeting mode.
	InvalidTarget,

	// The target lies beyond the miracle's maximum casting range.
	OutOfRange,

	// The world does not contain enough influence to pay the casting cost.
	InsufficientInfluence,

	// The miracle has no effect class or its effect actor could not be spawned.
	SpawnFailed
};

// Runtime state for one villager need. 1.0 = fully satisfied; 0.0 = critical.
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
	 * @param NeedIn The need represented by this state.
	 * @param ValueIn The initial satisfaction value.
	 * @param DecayPerSecondIn The satisfaction lost per second before global scaling.
	 */
	FGodGameNeedState(EVillagerNeed NeedIn, float ValueIn, float DecayPerSecondIn)
	: Need(NeedIn), Value(ValueIn), DecayPerSecond(DecayPerSecondIn)
	{	}

	// Need represented by this state. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need")
	EVillagerNeed Need = EVillagerNeed::Hunger;

	// Current satisfaction in the inclusive range [0, 1]. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Value = 1.0f;

	// Satisfaction lost per second before the component's decay multiplier is applied. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Need", meta = (ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float DecayPerSecond = 0.01f;
};

// Coarse village resource state whose tag allows content-only resource types.
USTRUCT(BlueprintType)
struct GODGAME_API FGodGameResourceState
{
	GENERATED_BODY()

	// Gameplay tag that uniquely identifies the resource type. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Resource", meta = (GameplayTagFilter="GodGame.Resource"))
	FGameplayTag ResourceTag;

	// Current stored amount, clamped between zero and Capacity by component operations. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Resource", meta = (ClampMin="0.0", UIMin="0.0"))
	float Amount = 0.0f;

	// Maximum amount that can be stored. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Resource", meta = (ClampMin="0.0", UIMin="0.0"))
	float Capacity = 100.0f;

	// Positive values produce resource over time; negative values consume it. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Resource")
	float PassiveDeltaPerSecond = 0.0f;
};

// Structured result that allows previews and UI to explain whether a miracle can be cast.
USTRUCT(BlueprintType)
struct GODGAME_API FGodGameMiracleCastCheck
{
	GENERATED_BODY()

	// Whether all targeting, cooldown, influence, and spawn-class checks passed.
	UPROPERTY(BlueprintReadOnly, Category = "Miracle")
	uint8 bCanCast : 1 = false;

	// Reason the cast failed or None if it succeeded.
	UPROPERTY(BlueprintReadOnly, Category = "Miracle")
	EGodGameMiracleCastFailure FailureReason = EGodGameMiracleCastFailure::None;

	// Localized user-facing explanation of the failure.
	UPROPERTY(BlueprintReadOnly, Category = "Miracle")
	FText Message = FText();
};
