// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Utilities/GodGameBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "Systems/SystemicWorldSubsystem.h"
#include "Systems/Events/SystemicEvent.h"
#include "Systems/Traits/SystemicTraitComponent.h"

#include "Interaction/GodGameReactionInterface.h"
#include "Simulation/FaithComponent.h"
#include "Simulation/GodGameWorldSubsystem.h"
#include "Simulation/VillageResourceComponent.h"
#include "Simulation/VillagerNeedsComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameBlueprintLibrary)

// TODO (trent, 8/24/26): Rewrite this entire file at some point.

namespace GodGameBlueprintLibraryPrivate
{
	static bool PassesTraitFilter(const AActor* Actor, const FGameplayTagContainer& RequiredTraits)
	{
		if(RequiredTraits.IsEmpty())
		{
			return true;
		}

		const USystemicTraitComponent* pTraits = IsValid(Actor) ? Actor->FindComponentByClass<USystemicTraitComponent>() : nullptr;
		return(pTraits && pTraits->GetTraits().HasAll(RequiredTraits));
	}

	template <typename FuncType>
	static int32 ForEachActorInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, const FGameplayTagContainer& RequiredTraits, FuncType&& Func)
	{
		UWorld* pWorld = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		if(!pWorld || Radius < 0.0f)
		{
			return 0;
		}

		const float RadiusSquared = FMath::Square(Radius);
		int32 AffectedCount = 0;

		// TActorIterator is perfectly adequate for a small portfolio prototype and makes effect actors independent of collision/object channel setup. Replace with spatial queries if the world grows large.
		for(TActorIterator<AActor> It(pWorld); It; ++It)
		{
			AActor* Actor = *It;
			if(!IsValid(Actor) || (FVector::DistSquared(Origin, Actor->GetActorLocation()) > RadiusSquared) || !PassesTraitFilter(Actor, RequiredTraits))
			{
				continue;
			}

			if(Func(Actor))
			{
				++AffectedCount;
			}
		}

		return AffectedCount;
	}
}

// Accessor for the GodGame subsystem.
UGodGameWorldSubsystem* UGodGameBlueprintLibrary::GetGodGameWorldSubsystem(const UObject* WorldContextObject)
{
	UWorld* pWorld = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return pWorld ? pWorld->GetSubsystem<UGodGameWorldSubsystem>() : nullptr;
}

// Retrieves all actors within a specified radius that possess a SystemicTraitComponent and match the given traits.
TArray<AActor*> UGodGameBlueprintLibrary::GetGodGameActorsInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, const FGameplayTagContainer& RequiredTraits)
{
	TArray<AActor*> Result;
	
	GodGameBlueprintLibraryPrivate::ForEachActorInRadius(WorldContextObject, Origin, Radius, RequiredTraits,
	                                                     [&Result](AActor* Actor)
	                                                     {
		                                                     if(Actor->FindComponentByClass<USystemicTraitComponent>())
		                                                     {
			                                                     Result.Add(Actor);
			                                                     return true;
		                                                     }
	                                                     	
		                                                     return false;
	                                                     });
	
	return Result;
}

// Modifies the specified need by a given delta for all actors within a given radius that meet the required traits.
int32 UGodGameBlueprintLibrary::ModifyNeedInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, EVillagerNeed Need, float Delta, const FGameplayTagContainer& RequiredTraits)
{
	return GodGameBlueprintLibraryPrivate::ForEachActorInRadius(WorldContextObject, Origin, Radius, RequiredTraits,
	                                                            [Need, Delta](AActor* Actor)
	                                                            {
		                                                            if(UVillagerNeedsComponent* pVillagerNeeds = Actor->FindComponentByClass<UVillagerNeedsComponent>())
		                                                            {
			                                                            pVillagerNeeds->ModifyNeed(Need, Delta);
			                                                            return true;
		                                                            }

	                                                            	return false;
	                                                            });
}

// Applies a faith delta to matching actors with FaithComponent. Returns affected actor count.
int32 UGodGameBlueprintLibrary::ModifyFaithInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, float Delta, const FGameplayTagContainer& RequiredTraits)
{
	return GodGameBlueprintLibraryPrivate::ForEachActorInRadius(WorldContextObject, Origin, Radius, RequiredTraits,
	                                                            [Delta](AActor* Actor)
	                                                            {
		                                                            if(UFaithComponent* pFaith = Actor->FindComponentByClass<UFaithComponent>())
		                                                            {
			                                                            pFaith->ModifyFaith(Delta);
			                                                            return true;
		                                                            }
		                                                            return false;
	                                                            });
}

// Adds/consumes a tagged village resource on matching actors with VillageResourceComponent.
int32 UGodGameBlueprintLibrary::ModifyResourceInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, FGameplayTag ResourceTag, float Delta, const FGameplayTagContainer& RequiredTraits)
{
	return GodGameBlueprintLibraryPrivate::ForEachActorInRadius(WorldContextObject, Origin, Radius, RequiredTraits,
	                                                            [ResourceTag, Delta](AActor* Actor)
	                                                            {
		                                                            if(UVillageResourceComponent* pVillageResources = Actor->FindComponentByClass<UVillageResourceComponent>())
		                                                            {
			                                                            pVillageResources->AddResource(ResourceTag, Delta);
			                                                            return true;
		                                                            }
	                                                            	
		                                                            return false;
	                                                            });
}

// Emits a JoyCore systemic event and invokes the legacy Blueprint reaction interface on matching actors.
int32 UGodGameBlueprintLibrary::SendReactionInRadius(const UObject* WorldContextObject, FVector Origin, float Radius, FGameplayTag ReactionTag, float Strength, AActor* SourceActor, const FGameplayTagContainer& RequiredTraits)
{
	if(!ReactionTag.IsValid())
	{
		return 0;
	}

	return GodGameBlueprintLibraryPrivate::ForEachActorInRadius(WorldContextObject, Origin, Radius, RequiredTraits,
	                                                            [ReactionTag, Strength, Origin, SourceActor](
	                                                            AActor* Actor)
	                                                            {
		                                                            FSystemicEvent Event;
		                                                            Event.EventTag = ReactionTag;
		                                                            Event.Subject = ESystemicEventSubject::Target;
		                                                            Event.Source = SourceActor;
		                                                            Event.Instigator = SourceActor;
		                                                            Event.Target = Actor;

		                                                            FSystemicEventData& EventData = Event.GetEventDataMutable<FSystemicEventData>();
		                                                            EventData.Location = Origin;
		                                                            EventData.Value = Strength;

		                                                            const bool bEmittedSystemicEvent = USystemicWorldSubsystem::EmitEvent(Actor, Event);
		                                                            bool bInvokedLegacyReaction = false;

	                                                            	if(Actor->GetClass()->ImplementsInterface(UGodGameReactionInterface::StaticClass()) && IGodGameReactionInterface::Execute_CanReceiveGodGameReaction(Actor, ReactionTag, SourceActor))
		                                                            {
			                                                            IGodGameReactionInterface::Execute_ReceiveGodGameReaction(Actor, ReactionTag, Strength, Origin, SourceActor);
			                                                            bInvokedLegacyReaction = true;
		                                                            }

		                                                            return(bEmittedSystemicEvent || bInvokedLegacyReaction);
	                                                            });
}
