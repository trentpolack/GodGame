// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/GodGameWorldSubsystem.h"

#include "Core/GodGameSettings.h"
#include "Simulation/CharacterFaithComponent.h"
#include "GodGameNativeGameplayTags.h"
#include "Systems/SystemicWorldSubsystem.h"
#include "Systems/Events/EventData/SystemicScalarEventData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameWorldSubsystem)

// Refreshes population metrics and applies passive and faith-based influence generation.
void UGodGameWorldSubsystem::RunSimulationStep(float DeltaSeconds)
{
    RefreshFaithMetrics();

    const float influenceGeneratedPerSecond = PassiveInfluencePerSecond + (bGenerateInfluenceFromFaith ? FaithInfluencePerSecond : 0.0f);
    if(!FMath::IsNearlyZero(influenceGeneratedPerSecond))
    {
        AddInfluence(influenceGeneratedPerSecond*DeltaSeconds);
    }
}

// Recalculates believer count, average faith, and faith influence generation.
void UGodGameWorldSubsystem::RefreshFaithMetrics()
{
    int32 ValidCount = 0;
    int32 BelieverCountNew = 0;
    float FaithSum = 0.0f;
    float InfluenceRateNew = 0.0f;

    // Get the current state of the simulation to avoid broadcasting change events if there is no actual change.
    int32 BelieverCountOld = BelieverCount;
    float AverageFaithOld = AverageFaith;
    float FaithInfluencePerSecondOld = FaithInfluencePerSecond;

    for(auto It = FaithComponents.CreateIterator(); It; ++It)
    {
        UCharacterFaithComponent* pFaithComponent = It->Get();
        if (!IsValid(pFaithComponent))
        {
            It.RemoveCurrent();
            continue;
        }

        ++ValidCount;
        FaithSum+= pFaithComponent->GetFaith();
        InfluenceRateNew+= pFaithComponent->GetInfluenceGenerationRate();

        if(pFaithComponent->IsBeliever())
        {
            ++BelieverCountNew;
        }
    }

    // Update the simulation state.
    BelieverCount = BelieverCountNew;
    AverageFaith = (ValidCount > 0) ? (FaithSum/ValidCount) : 0.0f;
    FaithInfluencePerSecond = InfluenceRateNew;

    // Check for a change in the simulation state before broadcasting to ensure there was a change.
    if((BelieverCount != BelieverCountOld) || (AverageFaith != AverageFaithOld) || (FaithInfluencePerSecond != FaithInfluencePerSecondOld))
    {
        // Broadcast faith metrics changed.
        OnFaithMetricsChanged.Broadcast(BelieverCount, AverageFaith, FaithInfluencePerSecond);
    }
}

// Influence accessor.
float UGodGameWorldSubsystem::GetInfluence() const
{
    return Influence;
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
void UGodGameWorldSubsystem::RegisterFaithComponent(UCharacterFaithComponent* FaithComponent)
{
    if(IsValid(FaithComponent))
    {
        FaithComponents.Add(FaithComponent);
    }
}

// Removes a faith component from population aggregation.
void UGodGameWorldSubsystem::UnregisterFaithComponent(UCharacterFaithComponent* FaithComponent)
{
    FaithComponents.Remove(TWeakObjectPtr<UCharacterFaithComponent>(FaithComponent));
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

    Collection.InitializeDependency<USystemicWorldSubsystem>();
	if(USystemicWorldSubsystem* Systems = GetWorld()->GetSubsystem<USystemicWorldSubsystem>())
	{
		Systems->AddEventStructMapping(TAG_GodGame_Event_Need_Changed, FSystemicScalarEventData::StaticStruct());
		Systems->AddEventStructMapping(TAG_GodGame_Event_Need_Critical, FSystemicScalarEventData::StaticStruct());
		Systems->AddEventStructMapping(TAG_GodGame_Event_Need_Recovered, FSystemicScalarEventData::StaticStruct());
	}

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
