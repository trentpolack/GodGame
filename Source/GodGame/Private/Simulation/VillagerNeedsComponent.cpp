// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/VillagerNeedsComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(VillagerNeedsComponent)

// Constructor.
UVillagerNeedsComponent::UVillagerNeedsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Adds a signed delta to a need, creating its state when absent and clamping to [0.0, 1.0].
float UVillagerNeedsComponent::ModifyNeed(EVillagerNeed Need, float Delta)
{
    if(FGodGameNeedState* pState = FindNeed(Need))
    {
        return(SetNeedValue(Need, pState->Value + Delta));
    }

    // Missing entries are created on demand, which is convenient for content-side experiments.
    FGodGameNeedState& NewState = Needs.AddDefaulted_GetRef();
    NewState.Need = Need;
    NewState.Value = FMath::Clamp(Delta, 0.0f, 1.0f);
    OnNeedChanged.Broadcast(Need, NewState.Value, NewState.Value);

    return NewState.Value;
}

// Replaces a need's value, creating its state when absent and clamping to [0.0, 1.0].
float UVillagerNeedsComponent::SetNeedValue(EVillagerNeed Need, float ValueNew)
{
    if(FGodGameNeedState* State = FindNeed(Need))
    {
        const float OldValue = State->Value;
        State->Value = FMath::Clamp(ValueNew, 0.0f, 1.0f);

        const float Delta = State->Value - OldValue;
        if (!FMath::IsNearlyZero(Delta))
        {
            OnNeedChanged.Broadcast(Need, State->Value, Delta);
        }

        return State->Value;
    }

    FGodGameNeedState& NewState = Needs.AddDefaulted_GetRef();
    NewState.Need = Need;
    NewState.Value = FMath::Clamp(ValueNew, 0.0f, 1.0f);
    OnNeedChanged.Broadcast(Need, NewState.Value, NewState.Value);
    return NewState.Value;
}

// Retrieves a need's current satisfaction.
float UVillagerNeedsComponent::GetNeedValue(EVillagerNeed Need) const
{
    if(const FGodGameNeedState* pState = FindNeed(Need))
    {
        return pState->Value;
    }

    // Need does not exist.
    return 0.0f;
}

// Converts satisfaction to an urgency score suitable for StateTree or utility decisions.
float UVillagerNeedsComponent::GetNeedUrgency(EVillagerNeed Need) const
{
    return(1.0f - GetNeedValue(Need));
}

// Finds the configured need with the lowest satisfaction value.
EVillagerNeed UVillagerNeedsComponent::GetMostUrgentNeed() const
{
    EVillagerNeed MostUrgentNeed = EVillagerNeed::Hunger;
    float LowestValue = TNumericLimits<float>::Max();

    for (const FGodGameNeedState& State : Needs)
    {
        if (State.Value < LowestValue)
        {
            LowestValue = State.Value;
            MostUrgentNeed = State.Need;
        }
    }

    return MostUrgentNeed;
}

// Replaces Needs with tuned prototype states for Hunger, Rest, Safety, and Faith.
void UVillagerNeedsComponent::ResetToPrototypeDefaults()
{
    // TODO (trent, 8/24/26): This is bad.
    Needs = {
        { EVillagerNeed::Hunger, 1.0f, 0.012f },
        { EVillagerNeed::Rest,   1.0f, 0.008f },
        { EVillagerNeed::Safety, 1.0f, 0.004f },
        { EVillagerNeed::Faith,  0.5f, 0.003f }
    };
}

// Finds a mutable need state by enum value.
FGodGameNeedState* UVillagerNeedsComponent::FindNeed(EVillagerNeed Need)
{
    return(Needs.FindByPredicate([Need](const FGodGameNeedState& State) { return State.Need == Need; }));
}

// Finds a read-only need state by enum value.
const FGodGameNeedState* UVillagerNeedsComponent::FindNeed(EVillagerNeed Need) const
{
    return(Needs.FindByPredicate([Need](const FGodGameNeedState& State) { return State.Need == Need; }));
}

// Begin play.
void UVillagerNeedsComponent::BeginPlay()
{
    Super::BeginPlay();

    PrimaryComponentTick.TickInterval = FMath::Max(0.05f, UpdateInterval);

    if (bInitializePrototypeDefaultsIfEmpty && Needs.IsEmpty())
    {
        ResetToPrototypeDefaults();
    }
}

// Component tick.
void UVillagerNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bDecayNeeds || DecayMultiplier <= 0.0f)
    {
        return;
    }

    // Update need states.
    for (FGodGameNeedState& State : Needs)
    {
        const float OldValue = State.Value;
        State.Value = FMath::Clamp(State.Value - (State.DecayPerSecond*DecayMultiplier*DeltaTime), 0.0f, 1.0f);

        const float Delta = State.Value - OldValue;
        if (!FMath::IsNearlyZero(Delta))
        {
            OnNeedChanged.Broadcast(State.Need, State.Value, Delta);
        }
    }
}
