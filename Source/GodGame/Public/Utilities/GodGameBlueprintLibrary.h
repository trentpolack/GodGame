// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Core/GodGameTypes.h"

#include "GodGameBlueprintLibrary.generated.h"

class UGodGameWorldSubsystem;

/** Blueprint helper library for querying and modifying God Game simulation state. */
UCLASS()
class GODGAME_API UGodGameBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Accessor for the GodGame subsystem.
	 * @param WorldContextObject The context object to get the subsystem for.
	 * @return Pointer to the GodGameWorldSubsystem instance.
	 */
	UFUNCTION(BlueprintPure, Category = "GodGame", meta = (WorldContext="WorldContextObject"))
	static UGodGameWorldSubsystem* GetGodGameWorldSubsystem(const UObject* WorldContextObject);

	/**
	 * Retrieves all actors within a specified radius that possess a SystemicTraitComponent and match the given traits.
	 * @param WorldContextObject The context object from which to retrieve the actors. Typically, the world or another relevant context.
	 * @param Origin The center location of the search radius.
	 * @param Radius The radius within which to search for actors.
	 * @param RequiredTraits The gameplay tag container specifying the traits that the actors must possess to be included in the results.
	 * @return Array of actors that meet the criteria.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Gameplay", meta = (WorldContext="WorldContextObject", GameplayTagFilter="System.Trait", AutoCreateRefTerm="RequiredTraits"))
	static TArray<AActor*> GetGodGameActorsInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, const FGameplayTagContainer& RequiredTraits);

	/**
	 * Modifies the specified need by a given delta for all actors within a given radius that meet the required traits.
	 * @param WorldContextObject The context object used to determine the relevant world.
	 * @param Origin The center location of the search radius.
	 * @param Radius The radius within which to search for actors.
	 * @param NeedTag The need to be modified (e.g., Hunger, Rest, Safety, Faith).
	 * @param Delta The amount by which to modify the need. Positive values increase satisfaction, negative values decrease it.
	 * @param RequiredTraits The gameplay tag container specifying traits that actors must possess to be considered.
	 * @return The number of actors affected by the modification.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Gameplay", meta = (WorldContext="WorldContextObject", AutoCreateRefTerm="RequiredTraits"))
	static int32 ModifyNeedInRadius(const UObject* WorldContextObject, FVector Origin, float Radius,
									UPARAM(meta = (GameplayTagFilter="GodGame.Need")) const FGameplayTag& NeedTag, float Delta,
									UPARAM(meta = (GameplayTagFilter="System.Trait")) const FGameplayTagContainer& RequiredTraits);

	/**
	 * Adds/consumes a tagged village resource on matching actors with VillageResourceComponent.
	 * @param WorldContextObject The context object to use for accessing the world.
	 * @param Origin The center location of the search radius.
	 * @param Radius The radius within which to search for actors.
	 * @param ResourceTag The gameplay tag specifying the resource to modify.
	 * @param Delta The amount by which to modify the resource. Positive values increase the resource, negative values decrease it.
	 * @param RequiredTraits The gameplay tag container specifying traits that actors must possess to be considered.
	 * @return The number of actors affected by the resource modification.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Gameplay", meta = (WorldContext="WorldContextObject", AutoCreateRefTerm="RequiredTraits"))
	static int32 ModifyResourceInRadius(const UObject* WorldContextObject, FVector Origin, float Radius,
	                                    UPARAM(meta = (GameplayTagFilter="GodGame.Resource")) const FGameplayTag& ResourceTag, float Delta,
	                                    UPARAM(meta = (GameplayTagFilter="System.Trait")) const FGameplayTagContainer& RequiredTraits);

	/**
	 * Emits a JoyCore systemic event and invokes the legacy Blueprint reaction interface on matching actors.
	 * @param WorldContextObject The context object to use for accessing the world.
	 * @param Origin The center of the radius in which to search for actors.
	 * @param Radius The radius around the origin to include actors.
	 * @param ReactionTag The gameplay tag associated with the reaction to emit.
	 * @param Strength The intensity or strength of the reaction.
	 * @param SourceActor The actor that serves as the source of the reaction.
	 * @param RequiredTraits The traits required for actors to be considered valid targets.
	 * @return The number of actors that successfully received the reaction event.
	 */
	UFUNCTION(BlueprintCallable, Category = "GodGame|Gameplay", meta = (WorldContext="WorldContextObject", AutoCreateRefTerm="RequiredTraits", DefaultToSelf="SourceActor"))
	static int32 SendReactionInRadius(const UObject* WorldContextObject, FVector Origin, float Radius,
	                                  UPARAM(meta = (GameplayTagFilter="System.Event.GodGame.Reaction")) const FGameplayTag& ReactionTag, float Strength, AActor* SourceActor,
	                                  UPARAM(meta = (GameplayTagFilter="System.Trait")) const FGameplayTagContainer& RequiredTraits);
};
