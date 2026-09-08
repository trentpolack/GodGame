// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Simulation/VillageResourceComponent.h"

#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(VillageResourceComponent)

// TODO (trent, 8/24/26): Generalize this more to support a variety of resources and remove hard-coded tags.

// Constructor.
UVillageResourceComponent::UVillageResourceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Finds a mutable resource state by exact tag.
FGodGameResourceState* UVillageResourceComponent::FindResource(const FGameplayTag& ResourceTag)
{
    return(Resources.FindByPredicate([ResourceTag](const FGodGameResourceState& State)
        {
            return(State.ResourceTag.MatchesTagExact(ResourceTag));
        }));
}

// Finds a read-only resource state by exact tag.
const FGodGameResourceState* UVillageResourceComponent::FindResource(const FGameplayTag& ResourceTag) const
{
    return(Resources.FindByPredicate([ResourceTag](const FGodGameResourceState& State)
        {
            return(State.ResourceTag.MatchesTagExact(ResourceTag));
        }));
}

// Adds a signed amount to a resource, creating its state when absent and clamping to capacity.
float UVillageResourceComponent::AddResource(const FGameplayTag& ResourceTag, float Amount)
{
    FGodGameResourceState* Resource = FindResource(ResourceTag);
    if(!Resource)
    {
        FGodGameResourceState& NewResource = Resources.AddDefaulted_GetRef();
        NewResource.ResourceTag = ResourceTag;
        Resource = &NewResource;
    }

    // Modify the resource amount.
    const float amountOld = Resource->Amount;
    Resource->Amount = FMath::Clamp(Resource->Amount + Amount, 0.0f, FMath::Max(0.0f, Resource->Capacity));
    const float resourceDelta = Resource->Amount - amountOld;

    if(!FMath::IsNearlyZero(resourceDelta))
    {
        OnResourceChanged.Broadcast(ResourceTag, Resource->Amount, resourceDelta);
    }

    return Resource->Amount;
}

// Attempts to consume a nonnegative resource amount atomically.
bool UVillageResourceComponent::TryConsumeResource(const FGameplayTag& ResourceTag, float Amount)
{
    if(Amount <= 0.0f)
    {
        // No change.
        return false;
    }

    FGodGameResourceState* pResource = FindResource(ResourceTag);
    if (!pResource || (pResource->Amount < Amount))
    {
        // Invalid or insufficient resources.
        return false;
    }

    // Modify the resource amount.
    AddResource(ResourceTag, -Amount);
    return true;
}

// Replaces a resource amount, creating its state when absent and clamping to capacity
float UVillageResourceComponent::SetResourceAmount(const FGameplayTag& ResourceTag, float AmountNew)
{
    FGodGameResourceState* pResource = FindResource(ResourceTag);
    if (!pResource)
    {
        FGodGameResourceState& NewResource = Resources.AddDefaulted_GetRef();
        NewResource.ResourceTag = ResourceTag;
        pResource = &NewResource;
    }

    // Set the resource amount to the specified value, clamping it to the resource's capacity.
    const float amountOld = pResource->Amount;
    pResource->Amount = FMath::Clamp(AmountNew, 0.0f, FMath::Max(0.0f, pResource->Capacity));

    const float amountDelta = pResource->Amount - amountOld;
    if(!FMath::IsNearlyZero(amountDelta))
    {
        // Broadcast the resource change event.
        OnResourceChanged.Broadcast(ResourceTag, pResource->Amount, amountDelta);
    }

    return pResource->Amount;
}

// Retrieves the current amount of a resource.
float UVillageResourceComponent::GetResourceAmount(const FGameplayTag& ResourceTag) const
{
    const FGodGameResourceState* pResource = FindResource(ResourceTag);
    return(pResource ? pResource->Amount : 0.0f);
}

// Retrieves resource storage as a fraction of capacity
float UVillageResourceComponent::GetResourceNormalized(const FGameplayTag& ResourceTag) const
{
    const FGodGameResourceState* pResource = FindResource(ResourceTag);
    return(pResource && (pResource->Capacity > 0.0f ? (pResource->Amount/pResource->Capacity) : 0.0f));
}

// Replaces Resources with Food and Wood entries.
void UVillageResourceComponent::ResetToDefaults()
{
    Resources.Reset();

    // TODO (trent 8/24/26): Convert to native gameplay tags and also this function shouldn't exist.
    const FGameplayTag FoodTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GodGame.Resource.Food")), false);
    const FGameplayTag WoodTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GodGame.Resource.Wood")), false);

    // Add default food resource values.
    FGodGameResourceState Food;
    Food.ResourceTag = FoodTag;
    Food.Amount = 35.0f;
    Food.Capacity = 100.0f;
    Food.PassiveDeltaPerSecond = 0.0f;
    Resources.Add(Food);

    // Add default wood resource values.
    FGodGameResourceState Wood;
    Wood.ResourceTag = WoodTag;
    Wood.Amount = 20.0f;
    Wood.Capacity = 100.0f;
    Wood.PassiveDeltaPerSecond = 0.0f;
    Resources.Add(Wood);
}

// Begin play.
void UVillageResourceComponent::BeginPlay()
{
    Super::BeginPlay();

    PrimaryComponentTick.TickInterval = FMath::Max(0.1f, UpdateInterval);
}

// Component tick.
void UVillageResourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if(!bApplyPassiveResourceDeltas)
    {
        return;
    }

    for(FGodGameResourceState& Resource : Resources)
    {
        if(!FMath::IsNearlyZero(Resource.PassiveDeltaPerSecond))
        {
            AddResource(Resource.ResourceTag, Resource.PassiveDeltaPerSecond*DeltaTime);
        }
    }
}
