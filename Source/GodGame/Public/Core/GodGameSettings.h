// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DeveloperSettings.h"

#include "GodGameSettings.generated.h"

/**
 * Project-wide tuning exposed under Project Settings > Game > God Game Prototype.
 * These are deliberately coarse defaults; individual content assets/components can still override local behavior.
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName="God Game"))
class GODGAME_API UGodGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Influence available when a game world initializes.
	UPROPERTY(Config, EditAnywhere, Category = "Influence", meta = (ClampMin="0.0", UIMin="0.0"))
	float StartingInfluence = 40.0f;

	// Maximum influence that the world subsystem can store.
	UPROPERTY(Config, EditAnywhere, Category = "Influence", meta = (ClampMin="0.0", UIMin="0.0"))
	float MaxInfluence = 100.0f;

	// Baseline influence generated each second independently of population faith.
	UPROPERTY(Config, EditAnywhere, Category = "Influence")
	float PassiveInfluencePerSecond = 0.10f;

	// Whether registered believers contribute additional influence generation.
	UPROPERTY(Config, EditAnywhere, Category = "Influence")
	bool bGenerateInfluenceFromFaith = true;

	// Global simulation cadence; 0.25-1.0 seconds reasonable for now.
	UPROPERTY(Config, EditAnywhere, Category = "Simulation", meta = (Units="Seconds", ClampMin="0.05", UIMin="0.05"))
	float SimulationInterval = 0.5f;

	/**
	 * Returns the Project Settings category that contains the God Game settings.
	 * @return The Game settings category name.
	 */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
