// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/CharacterNeedsComponent.h"

#include "DrawDebugHelpers.h"
#include "GodGameNativeGameplayTags.h"
#include "Simulation/VillageResourceComponent.h"
#include "Systems/SystemicWorldSubsystem.h"
#include "Systems/Events/EventData/SystemicScalarEventData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterNeedsComponent)

// Default set of Needs and values/decay rates for each one.
TArray<FGodGameNeedState> UCharacterNeedsComponent::kBaseNeeds = {
	{TAG_GodGame_Need_Hunger, 0.0f, 0.012f},
	{TAG_GodGame_Need_Rest, 0.0f, 0.008f},
	{TAG_GodGame_Need_Fear, 0.0f, 0.004f},
	{TAG_GodGame_Need_Faith, 0.5f, 0.003f},
	{TAG_GodGame_Need_Socialization, 0.5f, 0.004f}
};

// Constructor.
UCharacterNeedsComponent::UCharacterNeedsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Adds a signed delta to a need, creating its state when absent and clamping to [0.0, 1.0].
float UCharacterNeedsComponent::ModifyNeed(const FGameplayTag& NeedTag, float Delta)
{
	return SetNeedValue(NeedTag, GetNeedValue(NeedTag) + Delta);
}

// Update state before notifications; never retain an array pointer across external callbacks.
float UCharacterNeedsComponent::SetNeedValue(const FGameplayTag& NeedTag, float ValueNew)
{
	const float needValueCurrent = GetNeedValue(NeedTag);
	if(!NeedTag.IsValid() || !FMath::IsNearlyEqual(needValueCurrent, ValueNew))
	{
		return(GetNeedValue(NeedTag));
	}

	// Get the mutable need state pointer.
	FGodGameNeedState* pState = FindNeedMutable(NeedTag);
	const float valuePrevious = pState ? pState->Value : 0.0f;
	const float valueNewClamped = FMath::Clamp(ValueNew, 0.0f, 1.0f);
	if(!pState)
	{
		pState = &Needs.AddDefaulted_GetRef();
		pState->Need = NeedTag;
	}
	pState->Value = valueNewClamped;

	// Update critical/recovery status.
	const bool bWasCritical = CriticalNeeds.HasTagExact(NeedTag);
	const bool bIsCritical = bWasCritical ? (valueNewClamped > RecoveryThreshold) : (valueNewClamped >= CriticalThreshold);
	if(bIsCritical)
	{
		CriticalNeeds.AddTag(NeedTag);
	}
	else
	{
		CriticalNeeds.RemoveTag(NeedTag);
	}

	if(bWasCritical != bIsCritical)
	{
		// The character has a critical need or the critical status has been recovered.
		EmitNeedEvent(bIsCritical ? TAG_GodGame_Event_Need_Critical : TAG_GodGame_Event_Need_Recovered, NeedTag, valuePrevious, valueNewClamped);
	}

	const float valueDelta = valueNewClamped - valuePrevious;
	if(!FMath::IsNearlyZero(valueDelta))
	{
		// Emit an event when the need value changes in the JoyCore sim.
		EmitNeedEvent(TAG_GodGame_Event_Need_Changed, NeedTag, valuePrevious, valueNewClamped);

		// Also broadcast the event to the UE event binding.
		OnNeedChanged.Broadcast(NeedTag, valueNewClamped, valueDelta);
	}

	return valueNewClamped;
}

void UCharacterNeedsComponent::EmitNeedEvent(const FGameplayTag& EventTag, const FGameplayTag& NeedTag, float PreviousValue, float NewValue)
{
	if(!HasBegunPlay() || !IsValid(GetOwner()))
	{
		return;
	}

	// Fill out the data for the event in the JoyCore system.
	FSystemicEvent Event;
	Event.EventTag = EventTag;
	Event.Target = GetOwner();
	Event.Source = this;
	Event.ContextTags.AddTag(NeedTag);
	Event.EventDataInstance.InitializeAs<FSystemicScalarEventData>();
	FSystemicScalarEventData& Data = Event.GetEventDataMutable<FSystemicScalarEventData>();
	Data.ScalarTag = NeedTag;
	Data.PreviousValue = PreviousValue;
	Data.NewValue = NewValue;
	Data.Value = NewValue - PreviousValue;
	Data.Location = GetOwner()->GetActorLocation();
	USystemicWorldSubsystem::EmitEvent(this, Event);
}

bool UCharacterNeedsComponent::IsNeedCritical(const FGameplayTag& NeedTag) const
{
	return(CriticalNeeds.HasTagExact(NeedTag));
}

// Get the character's overall wellbeing.
//	NOTE (trent, 9/8/26): Should give Need types a weight as being starving shouldn't be equal with starvation.
float UCharacterNeedsComponent::GetWellbeing() const
{
	if(Needs.IsEmpty())
	{
		// Invalid state.
		return 0.0f;
	}

	float wellbeing = 0.0f;
	for(const FGodGameNeedState& State : Needs)
	{
		wellbeing+= 1.0f - State.Value;
	}
	
	return(wellbeing/Needs.Num());
}

bool UCharacterNeedsComponent::TryEat(UVillageResourceComponent* Resources)
{
	if(bConsumingFood || !IsValid(Resources) || (FoodPerMeal <= 0.0f) || (HungerReliefPerMeal <= 0.0f) || (GetNeedValue(TAG_GodGame_Need_Hunger) <= 0.0f))
	{
		// Character is already consuming food, resources are invalid, or the character is not hungry.
		return false;
	}
	
	// Attempt to consume food.
	bConsumingFood = false;	
	if(!Resources->TryConsumeResource(TAG_GodGame_Resource_Food, FoodPerMeal))
	{
		return false;
	}

	// Successfully consumed food.
	//	TODO (trent, 9/8/26): Need to account for this state properly.
	bConsumingFood = true;
	ModifyNeed(TAG_GodGame_Need_Hunger, -HungerReliefPerMeal);
	return true;
}

// Retrieves a need's current satisfaction.
float UCharacterNeedsComponent::GetNeedValue(const FGameplayTag& NeedTag) const
{
	if(const FGodGameNeedState* pState = FindNeed(NeedTag))
	{
		return pState->Value;
	}

	// Need does not exist.
	return 0.0f;
}

// Converts satisfaction to an urgency score suitable for StateTree or utility decisions.
float UCharacterNeedsComponent::GetNeedUrgency(const FGameplayTag& NeedTag) const
{
	return (GetNeedValue(NeedTag));
}

// Finds the configured need with the most critical need.
FGameplayTag UCharacterNeedsComponent::GetMostUrgentNeed() const
{
	FGameplayTag MostUrgentNeed = FGameplayTag::EmptyTag;
	float HighestValue = -1.0f;

	// Go through all Needs and find the most critical one (i.e., highest value).
	for(const FGodGameNeedState& NeedState : Needs)
	{
		if(NeedState.Value > HighestValue)
		{
			HighestValue = NeedState.Value;
			MostUrgentNeed = NeedState.Need;
		}
	}

	return MostUrgentNeed;
}

// Replaces Needs with tuned prototype states for Hunger, Rest, Safety, and Faith.
void UCharacterNeedsComponent::ResetToDefaults()
{
	TArray<FGodGameNeedState> defaultNeeds = kBaseNeeds;
	for(const FGodGameNeedState& AdditionalNeed : AdditionalNeeds)
	{
		if(!AdditionalNeed.Need.IsValid() || !FMath::IsFinite(AdditionalNeed.Value) || !FMath::IsFinite(
			AdditionalNeed.DecayPerSecond))
		{
			continue;
		}

		// See if this need already exists; if it does, replace it with the override.
		FGodGameNeedState* pExistingNeed = defaultNeeds.FindByPredicate([&AdditionalNeed](const FGodGameNeedState& State)
			{
				return(State.Need == AdditionalNeed.Need);
			});

		if(pExistingNeed)
		{
			*pExistingNeed = AdditionalNeed;
		}
		else
		{
			defaultNeeds.Add(AdditionalNeed);
		}
	}

	Needs.Reset();
	CriticalNeeds.Reset();
	
	// Populate every state before any listener runs.
	for(FGodGameNeedState needState : defaultNeeds)
	{
		needState.Value = FMath::Clamp(needState.Value, 0.0f, 1.0f);
		needState.DecayPerSecond = FMath::Max(0.0f, needState.DecayPerSecond);
		Needs.Add(needState);

		if(needState.Value >= FMath::Clamp(CriticalThreshold, 0.0f, 1.0f))
		{
			// The need for this needs to be addressed urgently.
			CriticalNeeds.AddTag(needState.Need);
		}
	}
}

// Finds a mutable need state by tag.
FGodGameNeedState* UCharacterNeedsComponent::FindNeedMutable(const FGameplayTag& NeedTag)
{
	return(Needs.FindByPredicate([NeedTag](const FGodGameNeedState& NeedState)
		{
			return(NeedState.Need.MatchesTagExact(NeedTag));
		}));
}

// Finds a read-only need state by tag.
const FGodGameNeedState* UCharacterNeedsComponent::FindNeed(const FGameplayTag& NeedTag) const
{
	return(Needs.FindByPredicate([NeedTag](const FGodGameNeedState& NeedState)
		{
			return (NeedState.Need.MatchesTagExact(NeedTag));
		}));
}

// Begin play.
void UCharacterNeedsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set the tick interval.
	PrimaryComponentTick.TickInterval = FMath::Max(0.05f, UpdateInterval);

	// Set the Needs states to the default values.
	ResetToDefaults();
}

// Component tick.
void UCharacterNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bDecayNeeds && (DecayMultiplier > 0.0f) && (DeltaTime > 0.0f))
	{
		// Snapshot of tags/rates since Blueprint delegates can add needs or reset the array.
		const TArray<FGodGameNeedState> needsSnapshot = Needs;
		for(const FGodGameNeedState& needState : needsSnapshot)
		{
			ModifyNeed(needState.Need, (needState.DecayPerSecond*DeltaTime)*DecayMultiplier);
		}
	}

	if(IsValid(FoodSource) && (GetNeedValue(TAG_GodGame_Need_Hunger) >= FMath::Clamp(EatAtHunger, 0.0f, 1.0f)))
	{
		// Attempt to eat.
		// TODO (trent, 9/8/26): Data-drive this and integrate it into the Villager AI.
		TryEat(FoodSource);
	}

	// Debug draw if enabled.
	if(bDebugDraw)
	{
		if(AActor* Owner = GetOwner())
		{
			const FVector TextLocation = DebugDrawBaseOffset;

			// Draw the header.
			DrawDebugString(GetWorld(), TextLocation, TEXT("NEEDS"), Owner, FColor(255, 128, 255), PrimaryComponentTick.TickInterval * 1.1f, false, DebugDrawScale * 1.25f);

			// Draw the needs.
			uint32 Index = 1;
			for(const FGodGameNeedState& NeedState : Needs)
			{
				const float NeedValue = NeedState.Value;
				DrawDebugString(GetWorld(), TextLocation + (DebugDrawLineOffset*Index)	,
				                FString::Printf(TEXT("%s: %.2f"), *NeedState.Need.GetTagLeafName().ToString(), NeedValue), Owner,
				                FColor::MakeRedToGreenColorFromScalar(1.0f - NeedValue),
				                PrimaryComponentTick.TickInterval*1.1f, false, DebugDrawScale);

				++Index;
			}
		}
	}
}
