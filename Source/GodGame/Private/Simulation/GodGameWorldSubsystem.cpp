// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/GodGameWorldSubsystem.h"

#include "Core/GodGameSettings.h"
#include "Simulation/FaithComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameWorldSubsystem)

// Refreshes population metrics and applies passive and faith-based influence generation.
void UGodGameWorldSubsystem::RunSimulationStep(float StepSeconds)
{
    RefreshFaithMetrics();

    const float GeneratedPerSecond = PassiveInfluencePerSecond + (bGenerateInfluenceFromFaith ? FaithInfluencePerSecond : 0.0f);
    if (!FMath::IsNearlyZero(GeneratedPerSecond))
    {
        AddInfluence(GeneratedPerSecond * StepSeconds);
    }
}

// Recalculates believer count, average faith, and faith influence generation.
void UGodGameWorldSubsystem::RefreshFaithMetrics()
{
    int32 ValidCount = 0;
    int32 BelieverCountNew = 0;
    float FaithSum = 0.0f;
    float InfluenceRateNew = 0.0f;

    for(auto It = FaithComponents.CreateIterator(); It; ++It)
    {
        UFaithComponent* pFaithComponent = It->Get();
        if (!IsValid(pFaithComponent))
        {
            It.RemoveCurrent();
            continue;
        }

        ++ValidCount;
        FaithSum+= pFaithComponent->Faith;
        InfluenceRateNew+= pFaithComponent->GetInfluenceGenerationRate();

        if(pFaithComponent->IsBeliever())
        {
            ++BelieverCountNew;
        }
    }

    BelieverCount = BelieverCountNew;
    AverageFaith = (ValidCount > 0) ? (FaithSum/ValidCount) : 0.0f;
    FaithInfluencePerSecond = InfluenceRateNew;

    // Broadcast faith metrics changed.
    //  TODO (trent, 8/24/26): Ensure there actually *is* a change.
    OnFaithMetricsChanged.Broadcast(BelieverCount, AverageFaith, FaithInfluencePerSecond);
}

// Attempts to deduct influence without allowing a negative balance.
bool UGodGameWorldSubsystem::TrySpendInfluence(float Amount)
{
    if((Amount < 0.0f) || (Influence < Amount))
    {
        return false;
    }

    SetInfluence(Influence - Amount);
    return true;
}

// Adds a signed amount to influence and clamps the result to the configured range.
void UGodGameWorldSubsystem::AddInfluence(float Amount)
{
    SetInfluence(Influence + Amount);
}

// Replaces the influence balance with a clamped value and broadcasts any applied change.
void UGodGameWorldSubsystem::SetInfluence(float InfluenceNew)
{
    const float InfluenceOld = Influence;

    Influence = FMath::Clamp(InfluenceNew, 0.0f, MaxInfluence);
    const float Delta = Influence - InfluenceOld;

    if(!FMath::IsNearlyZero(Delta))
    {
        // Broadcast a change in influence.
        OnInfluenceChanged.Broadcast(Influence, Delta);
    }
}

// Return current influence remapped into the range [0, 1].
float UGodGameWorldSubsystem::GetInfluenceNormalized() const
{
    return((MaxInfluence > 0.0f) ? (Influence/MaxInfluence) : 0.0f);
}

// Adds a valid faith component to population aggregation.
void UGodGameWorldSubsystem::RegisterFaithComponent(UFaithComponent* FaithComponent)
{
    if(IsValid(FaithComponent))
    {
        FaithComponents.Add(FaithComponent);
    }
}

// Removes a faith component from population aggregation.
void UGodGameWorldSubsystem::UnregisterFaithComponent(UFaithComponent* FaithComponent)
{
    FaithComponents.Remove(TWeakObjectPtr<UFaithComponent>(FaithComponent));
}

// Determines whether the subsystem should be created.
bool UGodGameWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* pWorld = Cast<UWorld>(Outer);
    return(pWorld && pWorld->IsGameWorld());
}

// Initialize the subsystem.
void UGodGameWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize from default settings.
    const UGodGameSettings* Settings = GetDefault<UGodGameSettings>();
    SimulationInterval = FMath::Max(0.05f, Settings->SimulationInterval);
    MaxInfluence = FMath::Max(0.0f, Settings->MaxInfluence);
    Influence = FMath::Clamp(Settings->StartingInfluence, 0.0f, MaxInfluence);
    PassiveInfluencePerSecond = Settings->PassiveInfluencePerSecond;

    bGenerateInfluenceFromFaith = Settings->bGenerateInfluenceFromFaith;
}

// Tick the subsystem.
void UGodGameWorldSubsystem::Tick(float DeltaTime)
{
    SimulationAccumulator+= DeltaTime;

    // Coarse stepping avoids doing population aggregation every frame and makes tuning easier to reason about.
    while(SimulationAccumulator >= SimulationInterval)
    {
        RunSimulationStep(SimulationInterval);

        SimulationAccumulator-= SimulationInterval;
    }
}

// Get stat ID.
TStatId UGodGameWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UGodGameWorldSubsystem, STATGROUP_Tickables);
}
