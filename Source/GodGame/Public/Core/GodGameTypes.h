// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "GodGameTypes.generated.h"

//	TODO (trent, 8/24/26): Most of these should be able to be converted to gameplay tags eventually.

// Declarations.
class UGodGameMiracleDefinition;

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
