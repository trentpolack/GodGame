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

// Adds a signed delta to Faith and clamps the result to [0.0, 1.0].
float UCharacterFaithComponent::GetFaith() const
{
    UCharacterNeedsComponent* pNeedsComponent = GetOwner()->GetComponentByClass<UCharacterNeedsComponent>();
    if(!pNeedsComponent)
    {
		UE_LOG(LogGodGame, Error, TEXT("ERROR [UCharacterFaithComponent]: This requires a CharacterNeedsComponent on the owning actor: %hs"), __FUNCTION__);
        return 0.0f;
    }
    
    return(pNeedsComponent->GetNeedValue(TAG_GodGame_Need_Faith));
}

// Adds a signed delta to Faith and clamps the result to [0.0, 1.0].
float UCharacterFaithComponent::ModifyFaith(float Delta)
{
    return(SetFaith(GetFaith() + Delta));
}

// Replaces Faith with a clamped value and broadcasts changes.
float UCharacterFaithComponent::SetFaith(float FaithNew)
{
    // Get the needs component.
    UCharacterNeedsComponent* pNeedsComponent = GetOwner()->GetComponentByClass<UCharacterNeedsComponent>();
    if(!IsValid(pNeedsComponent))
    {
        UE_LOG(LogGodGame, Error, TEXT("ERROR [UCharacterFaithComponent]: This requires a CharacterNeedsComponent on the owning actor: %hs"), __FUNCTION__);
        return 0.0f;
    }
    
    // Update the character's Needs Component's faith (it's the source of truth).
    const float FaithOld = pNeedsComponent->GetNeedValue(TAG_GodGame_Need_Faith);
    const float Faith = pNeedsComponent->SetNeedValue(TAG_GodGame_Need_Faith, FMath::Clamp(FaithNew, 0.0f, 1.0f));

    const float Delta = Faith - FaithOld;
    if (!FMath::IsNearlyZero(Delta))
    {
        // Broadcast the faith change.
        OnFaithChanged.Broadcast(Faith, Delta, IsBeliever());
    }

    return Faith;
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
    return(bContributesInfluence && (IsBeliever() ? (GetFaith()*InfluenceGenerationPerSecondAtMaxFaith) : 0.0f));
}

// Begin play.
void UCharacterFaithComponent::BeginPlay()
{
    Super::BeginPlay();

    SetFaith(FMath::Clamp(GetFaith(), 0.0f, 1.0f));
    if(UGodGameWorldSubsystem* pSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
    {
        // Register this faith component.
        pSubsystem->RegisterFaithComponent(this);
    }
}

// End play.
void UCharacterFaithComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGodGameWorldSubsystem* Sim = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
    {
        Sim->UnregisterFaithComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}