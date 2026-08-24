// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Interaction/GodGameMiracleComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Data/GodGameMiracleDefinition.h"
#include "Interaction/GodGameMiraclePreviewActor.h"
#include "Simulation/GodGameWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameMiracleComponent)

// Constructor.
UGodGameMiracleComponent::UGodGameMiracleComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Resolves the location used for range checks.
FVector UGodGameMiracleComponent::GetCastOrigin() const
{
    const AActor* Owner = GetOwner();
    if(const APlayerController* pPlayerCharacter = Cast<APlayerController>(Owner))
    {
        if(const APawn* pPawn = pPlayerCharacter->GetPawn())
        {
            return pPawn->GetActorLocation();
        }
    }

    return(Owner ? Owner->GetActorLocation() : FVector::ZeroVector);
}

// Destroys the current preview and creates the preview class configured by SelectedMiracle, if any.
void UGodGameMiracleComponent::RecreatePreview()
{
    if(IsValid(PreviewActor))
    {
        PreviewActor->Destroy();
    }
    PreviewActor = nullptr;

    UWorld* pWorld = GetWorld();
    if(!SelectedMiracle || !SelectedMiracle->PreviewActorClass || !pWorld)
    {
        return;
    }

    // Prepare to spawn the preview actor.
    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PreviewActor = pWorld->SpawnActor<AGodGameMiraclePreviewActor>(SelectedMiracle->PreviewActorClass, FTransform::Identity, Params);
    if(IsValid(PreviewActor))
    {
        // Valid preview; update visibility.
        PreviewActor->SetActorHiddenInGame(true);
    }
}

// Selects a miracle and recreates its optional preview actor.
void UGodGameMiracleComponent::SelectMiracle(UGodGameMiracleDefinition* Miracle)
{
    if(SelectedMiracle == Miracle)
    {
        return;
    }

    SelectedMiracle = Miracle;
    RecreatePreview();

    // Broadcast the selected miracle change.
    OnSelectedMiracleChanged.Broadcast(SelectedMiracle);
}

// Clears the selected miracle and removes its preview actor.
void UGodGameMiracleComponent::ClearSelectedMiracle()
{
    SelectMiracle(nullptr);
}

// Calculates the cooldown time remaining for a miracle.
float UGodGameMiracleComponent::GetCooldownRemaining(const UGodGameMiracleDefinition* Miracle) const
{
    const UWorld* pWorld = GetWorld();
    if(!Miracle || (Miracle->CooldownSeconds <= 0.0f) || !pWorld)
    {
        return 0.0f;
    }

    const double* LastCastTime = LastCastTimes.Find(Miracle);
    if(!LastCastTime)
    {
        return 0.0f;
    }

    const double Elapsed = pWorld->GetTimeSeconds() - *LastCastTime;
    return(FMath::Max(0.0f, Miracle->CooldownSeconds - static_cast<float>(Elapsed)));
}

// Validates a prospective miracle cast without changing the gameplay state.
//  NOTE (trent, 8/24/26): Remove hard-coded failure reporting and strings.
FGodGameMiracleCastCheck UGodGameMiracleComponent::CanCastMiracle(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit) const
{
    FGodGameMiracleCastCheck Result;

    // Local failure reporting.
    //  TODO (trent, 8/24/26): Surface this.
    auto Fail = [&Result](EGodGameMiracleCastFailure Reason, const FText& Message)
    {
        Result.bCanCast = false;
        Result.FailureReason = Reason;
        Result.Message = Message;
    };

    if(!Miracle)
    {
        Fail(EGodGameMiracleCastFailure::NoMiracleSelected, NSLOCTEXT("GodGame", "NoMiracle", "No miracle selected."));
        return Result;
    }

    const bool bHasActorTarget = IsValid(Hit.GetActor());
    const bool bValidTarget = Hit.bBlockingHit && (Miracle->TargetingMode != EMiracleTargetingMode::Actor || bHasActorTarget);

    if(!bValidTarget)
    {
        Fail(EGodGameMiracleCastFailure::InvalidTarget, NSLOCTEXT("GodGame", "InvalidTarget", "Invalid miracle target."));
        return Result;
    }

    if((Miracle->MaxCastRange > 0.0f) && (FVector::DistSquared(GetCastOrigin(), Hit.ImpactPoint) > FMath::Square(Miracle->MaxCastRange)))
    {
        Fail(EGodGameMiracleCastFailure::OutOfRange, NSLOCTEXT("GodGame", "OutOfRange", "Target is out of range."));
        return Result;
    }

    if(GetCooldownRemaining(Miracle) > 0.0f)
    {
        Fail(EGodGameMiracleCastFailure::OnCooldown, NSLOCTEXT("GodGame", "OnCooldown", "Miracle is on cooldown."));
        return Result;
    }

    const UGodGameWorldSubsystem* pSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr;
    if(!pSubsystem || (pSubsystem->GetInfluence() < Miracle->InfluenceCost))
    {
        Fail(EGodGameMiracleCastFailure::InsufficientInfluence, NSLOCTEXT("GodGame", "NoInfluence", "Not enough influence."));
        return Result;
    }

    if(!IsValid(Miracle->MiracleActorClass))
    {
        Fail(EGodGameMiracleCastFailure::SpawnFailed, NSLOCTEXT("GodGame", "NoEffectClass", "Miracle has no effect actor class."));
        return Result;
    }

    // Can cast.
    Result.bCanCast = true;
    Result.FailureReason = EGodGameMiracleCastFailure::None;
    Result.Message = FText::GetEmpty();
    return Result;
}

// Attempts to spend influence and spawn a miracle effect at a cursor hit.
bool UGodGameMiracleComponent::CastMiracleFromHit(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit, AActor*& SpawnedActor, FGodGameMiracleCastCheck& OutResult)
{
    UWorld* pWorld = GetWorld();
    
    SpawnedActor = nullptr;
    OutResult = CanCastMiracle(Miracle, Hit);
    if(!OutResult.bCanCast || !pWorld)
    {
        return false;
    }

    UGodGameWorldSubsystem* Sim = pWorld->GetSubsystem<UGodGameWorldSubsystem>();
    if(!Sim || !Sim->TrySpendInfluence(Miracle->InfluenceCost))
    {
        OutResult.bCanCast = false;
        OutResult.FailureReason = EGodGameMiracleCastFailure::InsufficientInfluence;
        OutResult.Message = NSLOCTEXT("GodGame", "SpendFailed", "Not enough influence.");
        return false;
    }

    FRotator SpawnRotation = FRotator::ZeroRotator;
    if(Miracle->bAlignToSurfaceNormal)
    {
        SpawnRotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();
    }

    // Spawn the miracle actor class.
    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = GetOwner() ? GetOwner()->GetInstigator() : nullptr;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    SpawnedActor = pWorld->SpawnActor<AActor>(Miracle->MiracleActorClass, Hit.ImpactPoint, SpawnRotation, Params);
    if(!SpawnedActor)
    {
        // Do not charge the player for an effect actor that failed to spawn.
        Sim->AddInfluence(Miracle->InfluenceCost);
        OutResult.bCanCast = false;
        OutResult.FailureReason = EGodGameMiracleCastFailure::SpawnFailed;
        OutResult.Message = NSLOCTEXT("GodGame", "SpawnFailed", "Miracle effect failed to spawn.");
        return false;
    }

    LastCastTimes.Add(Miracle, pWorld->GetTimeSeconds());
    
    // Broadcast the miracle cast event.
    OnMiracleCast.Broadcast(Miracle, SpawnedActor);
    return true;
}

// Attempts to cast SelectedMiracle at a cursor hit.
bool UGodGameMiracleComponent::CastSelectedMiracleFromHit(const FHitResult& Hit, AActor*& SpawnedActor, FGodGameMiracleCastCheck& ResultOut)
{
    return(CastMiracleFromHit(SelectedMiracle, Hit, SpawnedActor, ResultOut));
}

// Updates the selected miracle's preview while the player is aiming, creating it lazily when needed.
void UGodGameMiracleComponent::UpdatePreview(const FHitResult& Hit)
{
    if(!SelectedMiracle)
    {
        HidePreview();
        return;
    }

    if(!IsValid(PreviewActor) && SelectedMiracle->PreviewActorClass)
    {
        RecreatePreview();
    }

    if(IsValid(PreviewActor))
    {
        PreviewActor->UpdatePreview(SelectedMiracle, Hit, CanCastMiracle(SelectedMiracle, Hit));
    }
}

// Hides the active preview actor without destroying it.
void UGodGameMiracleComponent::HidePreview()
{
    if(!IsValid(PreviewActor))
    {
        // No valid preview.
        return;
    }

    // Update preview visibility (is visible).
    PreviewActor->SetActorHiddenInGame(true);
}

// End play.
void UGodGameMiracleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Dispatch with the preview actor.
    if(IsValid(PreviewActor))
    {
        PreviewActor->Destroy();
    }
    PreviewActor = nullptr;

    Super::EndPlay(EndPlayReason);
}
