// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/CharacterFaithComponent.h"

#include "GodGame.h"
#include "Simulation/CharacterNeedsComponent.h"
#include "Simulation/GodGameWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterFaithComponent)

// Constructor.
UCharacterFaithComponent::UCharacterFaithComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Adds a signed delta to Faith and clamps the result to `[0.0, 1.0]`.
float UCharacterFaithComponent::GetFaith() const
{
	UCharacterNeedsComponent* pNeedsComponent = GetOwner() ? GetOwner()->GetComponentByClass<UCharacterNeedsComponent>() : nullptr;
	if(!pNeedsComponent)
	{
		UE_LOG(LogGodGame, Error, TEXT("ERROR [UCharacterFaithComponent]: This requires a CharacterNeedsComponent on the owning actor: %hs"), __FUNCTION__);
		return 0.0f;
	}

	return(1.0f - pNeedsComponent->GetNeedValue(FaithTag));
}

// Adds a signed delta to Faith and clamps the result to `[0.0, 1.0]`.
float UCharacterFaithComponent::ModifyFaith(float Delta)
{
	return(SetFaith(GetFaith() + Delta));
}

// Replaces Faith with a clamped value and broadcasts changes.
float UCharacterFaithComponent::SetFaith(float FaithNew)
{
	// Get the needs component.
	UCharacterNeedsComponent* pCharacterNeeds = GetOwner() ? GetOwner()->GetComponentByClass<UCharacterNeedsComponent>() : nullptr;
	if(!IsValid(pCharacterNeeds))
	{
		UE_LOG(LogGodGame, Error, TEXT("ERROR [UCharacterFaithComponent]: This requires a CharacterNeedsComponent on the owning actor: %hs"), __FUNCTION__);
		return 0.0f;
	}

	// Faith is `[0.0, 1.0]` where `0.0` is no faith and `1.0` is absolute faith; the Needs component's faith is the inverse.
	return(1.0f - pCharacterNeeds->SetNeedValue(FaithTag, 1.0f - FMath::Clamp(FaithNew, 0.0f, 1.0f)));
}

// Whether this component's owner is a believer. 
bool UCharacterFaithComponent::IsBeliever() const
{
	// A believer is someone who meets the minimum-specified faith threshold.
	return(GetFaith() >= BelieverThreshold);
}

// Calculates this component's current faith-based influence contribution.
float UCharacterFaithComponent::GetInfluenceGenerationRate() const
{
	return((bContributesInfluence && IsBeliever()) ? (GetFaith()*FMath::Max(0.0f, InfluenceGenerationPerSecondAtMaxFaith)) : 0.0f);
}

// Begin play.
void UCharacterFaithComponent::BeginPlay()
{
	Super::BeginPlay();

	if(UGodGameWorldSubsystem* pSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
	{
		// Register this faith component.
		pSubsystem->RegisterFaithComponent(this);
	}
}

// End play.
void UCharacterFaithComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(UGodGameWorldSubsystem* pSim = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
	{
		pSim->UnregisterFaithComponent(this);
	}

	Super::EndPlay(EndPlayReason);
}
