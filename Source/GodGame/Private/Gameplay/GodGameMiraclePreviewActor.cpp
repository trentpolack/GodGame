// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Gameplay/GodGameMiraclePreviewActor.h"

#include "Components/SceneComponent.h"

#include "Data/GodGameMiracleDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameMiraclePreviewActor)

// Constructor.
AGodGameMiraclePreviewActor::AGodGameMiraclePreviewActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SetActorEnableCollision(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
}

// Places and displays the preview for a prospective cast.
void AGodGameMiraclePreviewActor::UpdatePreview(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit, const FGodGameMiracleCastCheck& CastCheck)
{
    if(!Miracle)
    {
        SetActorHiddenInGame(true);
        return;
    }

    // Update visibility.
    SetActorHiddenInGame(!Hit.bBlockingHit);
    if(!Hit.bBlockingHit)
    {
        return;
    }

    // Update location and rotation.
    SetActorLocation(Hit.ImpactPoint);
    if(Miracle->bAlignToSurfaceNormal)
    {
        SetActorRotation(FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator());
    }
    else
    {
        SetActorRotation(FRotator::ZeroRotator);
    }

    // Call the blueprint event.
    OnPreviewUpdated(Miracle, Hit, CastCheck.bCanCast, CastCheck.FailureReason);
}
