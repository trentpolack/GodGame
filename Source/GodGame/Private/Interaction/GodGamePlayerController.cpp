// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Interaction/GodGamePlayerController.h"

#include "GameFramework/Pawn.h"

#include "Interaction/GodGameTagComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGamePlayerController)

// Constructor.
AGodGamePlayerController::AGodGamePlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

// Traces from the mouse cursor into the world using CursorTraceChannel.
bool AGodGamePlayerController::TraceGodCursor(FHitResult& HitOut, float TraceDistance) const
{
    const UWorld* pWorld = GetWorld();
    if(!pWorld)
    {
        return false;
    }

    // Deproject mouse position into world space.
    FVector WorldOrigin = FVector::ZeroVector;
    FVector WorldDirection = FVector::ZeroVector;
    if(!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
    {
        return false;
    }

    const float Distance = (TraceDistance > 0.0f) ? TraceDistance : DefaultTraceDistance;
    const FVector End = WorldOrigin + WorldDirection * Distance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(GodCursorTrace), true);
    if(const APawn* pPawn = GetPawn())
    {
        Params.AddIgnoredActor(pPawn);
    }

    return(pWorld->LineTraceSingleByChannel(HitOut, WorldOrigin, End, CursorTraceChannel, Params));
}

// Selects the actor under the cursor unless its God Game tag component opts out of selection.
AActor* AGodGamePlayerController::SelectActorUnderCursor()
{
    FHitResult Hit = FHitResult();
    AActor* pCandidate = TraceGodCursor(Hit) ? Hit.GetActor() : nullptr;

    // A tag component is optional, but if present it can explicitly opt an actor out of god selection.
    if(IsValid(pCandidate))
    {
        if(const UGodGameTagComponent* pTagComponent = pCandidate->FindComponentByClass<UGodGameTagComponent>())
        {
            if(!pTagComponent->bSelectable)
            {
                // Invalid selection candidate.
                pCandidate = nullptr;
            }
        }
    }

    SetSelectedActor(pCandidate);
    return SelectedActor;
}

// Replaces the selected actor and broadcasts the old and new selections when they differ.
void AGodGamePlayerController::SetSelectedActor(AActor* SelectionNew)
{
    if(SelectedActor == SelectionNew)
    {
        // Redundant.
        return;
    }

    AActor* pSelectionPrevious = SelectedActor;
    SelectedActor = SelectionNew;
    
    // Broadcast the change in selection.
    OnSelectionChanged.Broadcast(SelectedActor, pSelectionPrevious);
}

// Clears the current actor selection.
void AGodGamePlayerController::ClearSelection()
{
    SetSelectedActor(nullptr);
}
