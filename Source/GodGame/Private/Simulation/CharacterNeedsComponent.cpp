// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/CharacterNeedsComponent.h"

#include "DrawDebugHelpers.h"
#include "GodGameNativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterNeedsComponent)

// Default set of Needs and values/decay rates for each one.
TArray<FGodGameNeedState> UCharacterNeedsComponent::BaseNeeds = {
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
	if(FGodGameNeedState* pState = FindNeedMutable(NeedTag))
	{
		return (SetNeedValue(NeedTag, pState->Value + Delta));
	}

	// Missing entries are created on demand.
	FGodGameNeedState& NeedNew = Needs.AddDefaulted_GetRef();
	NeedNew.Need = NeedTag;
	NeedNew.Value = FMath::Clamp(Delta, 0.0f, 1.0f);
	OnNeedChanged.Broadcast(NeedTag, NeedNew.Value, NeedNew.Value);

	return NeedNew.Value;
}

// Replaces a need's value, creating its state when absent and clamping to [0.0, 1.0].
float UCharacterNeedsComponent::SetNeedValue(const FGameplayTag& NeedTag, float ValueNew)
{
	if(FGodGameNeedState* pState = FindNeedMutable(NeedTag))
	{
		const float ValueOld = pState->Value;
		pState->Value = FMath::Clamp(ValueNew, 0.0f, 1.0f);

		const float Delta = pState->Value - ValueOld;
		if(!FMath::IsNearlyZero(Delta))
		{
			// Only broadcast the new state if it's actually changed.
			OnNeedChanged.Broadcast(NeedTag, pState->Value, Delta);
		}

		return pState->Value;
	}

	// Add a new need type.
	FGodGameNeedState& NeedStateNew = Needs.AddDefaulted_GetRef();
	NeedStateNew.Need = NeedTag;
	NeedStateNew.Value = FMath::Clamp(ValueNew, 0.0f, 1.0f);

	// Broadcast the new value.
	OnNeedChanged.Broadcast(NeedTag, NeedStateNew.Value, NeedStateNew.Value);
	return NeedStateNew.Value;
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
	float HighestValue = TNumericLimits<float>::Min();

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
	// Empty the current array.
	Needs.Empty();

	// Reset the Needs states to the default values.
	Needs = BaseNeeds;

	// Go through the additional needs specified; if the Need tag exists already, then replace its settings, otherwise create a new Need.
	for(FGodGameNeedState& AdditionalNeed : AdditionalNeeds)
	{
		FGodGameNeedState* pNeed = FindNeedMutable(AdditionalNeed.Need);
		if(!pNeed)
		{
			// The most likely scenario; there are additional needs to add beyond the native set.
			Needs.Add(AdditionalNeed);

			continue;
		}

		// Update the existing need state with the additional need settings.
		pNeed->Value = AdditionalNeed.Value;
		pNeed->DecayPerSecond = AdditionalNeed.DecayPerSecond;
	}
}

// Finds a mutable need state by tag.
FGodGameNeedState* UCharacterNeedsComponent::FindNeedMutable(const FGameplayTag& NeedTag)
{
	return (Needs.FindByPredicate([NeedTag](const FGodGameNeedState& NeedState)
	{
		return (NeedState.Need.MatchesTag(NeedTag));
	}));
}

// Finds a read-only need state by tag.
const FGodGameNeedState* UCharacterNeedsComponent::FindNeed(const FGameplayTag& NeedTag) const
{
	return (Needs.FindByPredicate([NeedTag](const FGodGameNeedState& NeedState)
	{
		return (NeedState.Need.MatchesTag(NeedTag));
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

	if(!bDecayNeeds || DecayMultiplier <= 0.0f)
	{
		return;
	}

	// Build up a debug string.
	FString NeedsDebugString = FString();

	// Update need states.
	for(FGodGameNeedState& NeedState : Needs)
	{
		const float StateValueOld = NeedState.Value;
		NeedState.Value = FMath::Clamp(NeedState.Value + (NeedState.DecayPerSecond * DecayMultiplier * DeltaTime), 0.0f, 1.0f);

		const float Delta = NeedState.Value - StateValueOld;
		if(!FMath::IsNearlyZero(Delta))
		{
			// Broadcast the need state change.
			OnNeedChanged.Broadcast(NeedState.Need, NeedState.Value, Delta);
		}
	}

	// Debug draw if enabled.
	if(bDebugDraw)
	{
		if(AActor* Owner = GetOwner())
		{
			const FVector TextLocation = DebugDrawBaseOffset;
			
			// Draw the header.
			DrawDebugString(GetWorld(), TextLocation, TEXT("NEEDS"), Owner, FColor(255, 128, 255), PrimaryComponentTick.TickInterval*1.1f, false, DebugDrawScale*1.25f);
			
			// Draw the needs.
			uint32 Index = 1;
			for(const FGodGameNeedState& NeedState : Needs)
			{
				const float NeedValue = NeedState.Value;
				DrawDebugString(GetWorld(), TextLocation + DebugDrawLineOffset*Index, FString::Printf(TEXT("%s: %.2f\f"), *NeedState.Need.ToString(), NeedValue), Owner, FColor::MakeRedToGreenColorFromScalar(1.0f - NeedValue), PrimaryComponentTick.TickInterval*1.1f, false, DebugDrawScale);
				
				++Index;
			}
		}
	}
}
