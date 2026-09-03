// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/FaithComponent.h"

#include "Simulation/GodGameWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaithComponent)

// Constructor.
UFaithComponent::UFaithComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Adds a signed delta to Faith and clamps the result to [0.0, 1.0].
float UFaithComponent::ModifyFaith(float Delta)
{
    return(SetFaith(Faith + Delta));
}

// Replaces Faith with a clamped value and broadcasts changes.
float UFaithComponent::SetFaith(float FaithNew)
{
    const float FaithOld = Faith;
    Faith = FMath::Clamp(FaithNew, 0.0f, 1.0f);

    const float Delta = Faith - FaithOld;
    if (!FMath::IsNearlyZero(Delta))
    {
        // Broadcast the faith change.
        OnFaithChanged.Broadcast(Faith, Delta, IsBeliever());
    }

    return Faith;
}

// Calculates this component's current faith-based influence contribution.
float UFaithComponent::GetInfluenceGenerationRate() const
{
    return(bContributesInfluence && (IsBeliever() ? (Faith*InfluenceGenerationPerSecondAtMaxFaith) : 0.0f));
}

// Begin play.
void UFaithComponent::BeginPlay()
{
    Super::BeginPlay();

    Faith = FMath::Clamp(Faith, 0.0f, 1.0f);
    if(UGodGameWorldSubsystem* pSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
    {
        // Register this faith component.
        pSubsystem->RegisterFaithComponent(this);
    }
}

// End play.
void UFaithComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGodGameWorldSubsystem* Sim = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr)
    {
        Sim->UnregisterFaithComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}