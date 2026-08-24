// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Systems/Traits/SystemicTraitComponent.h"

#include "GodGameTagComponent.generated.h"

/**
 * God-game presentation and selection data layered on JoyCore's systemic trait provider.
 * Initial/runtime traits, trait mutation, change delegates, and systemic lifecycle events come from USystemicTraitComponent. Miracles can filter radius effects with those traits.
 *	TODO (trent, 8/24/26): Update this; seems unnecessary with some changes to JoyCore.
 */
UCLASS(ClassGroup = (GodGame), meta = (BlueprintSpawnableComponent))
class GODGAME_API UGodGameTagComponent : public USystemicTraitComponent
{
	GENERATED_BODY()

public:
	/** 
	 * Constructor.
	 */
	UGodGameTagComponent();

	// Display name presented for the component owner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Traits")
	FName DisplayName = NAME_None;

	// Whether God Game interaction may select the component owner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GodGame|Interaction")
	uint8 bSelectable : 1 = true;

	/**
	 * Returns the configured display name for the component owner.
	 * @return The owner's presentation name.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Traits")
	FName GetDisplayName() const;

	/**
	 * Returns whether the component owner may be selected.
	 * @return True when God Game interaction may select the owner.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame|Interactions")
	bool GetIsSelectable() const;
};
