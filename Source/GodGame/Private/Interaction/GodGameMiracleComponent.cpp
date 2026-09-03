// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Interaction/GodGameMiracleComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

#include "Data/GodGameMiracleDefinition.h"
#include "Interaction/GodGameMiraclePreviewActor.h"
#include "Simulation/GodGameWorldSubsystem.h"
#include "Utilities/GodGameBlueprintLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameMiracleComponent)

// Constants.
const float UGodGameMiracleComponent::DefaultMiracleRadius = 100.0f;
const float UGodGameMiracleComponent::DefaultMiracleHungerModifier = 0.25f;
const float UGodGameMiracleComponent::DefaultMiracleSafetyModifier = 0.10f;
const float UGodGameMiracleComponent::DefaultMiracleFaithModifier = 0.08f;

const FColor UGodGameMiracleComponent::DefaultMiracleDebugDrawColor = FColor(60, 170, 255);

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
    // If there's an existing preview, destroy it.
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
    if(!pWorld || !Miracle || (Miracle->CooldownSeconds <= 0.0f))
    {
        // Invalid miracle or the selected miracle is on cooldown.
        return 0.0f;
    }

    const double* LastCastTime = MiracleLastCastTimeMap.Find(Miracle);
    if(!LastCastTime)
    {
        // Miracle has not been cast yet.
        return 0.0f;
    }

    const double ElapsedTime = pWorld->GetTimeSeconds() - *LastCastTime;
    return(FMath::Max(0.0f, Miracle->CooldownSeconds - static_cast<float>(ElapsedTime)));
}

// Validates a prospective miracle cast without changing the gameplay state.
//  NOTE (trent, 8/24/26): Remove hard-coded failure reporting and strings.
FGodGameMiracleCastCheck UGodGameMiracleComponent::CanCastMiracle(UGodGameMiracleDefinition* Miracle, const FHitResult& Hit) const
{
    FGodGameMiracleCastCheck Result;

    // Local failure reporting.
    auto Fail = [&Result](const EGodGameMiracleCastFailure Reason, const FText& Message)
    {
        Result.bCanCast = false;
        Result.FailureReason = Reason;
        Result.Message = Message;
    };
    
    // Check valid Miracle and cooldown.
    if(!Miracle || (GetCooldownRemaining(Miracle) > 0.0f))
    {
        Fail(EGodGameMiracleCastFailure::NoMiracleSelectedOrOnCooldown, NSLOCTEXT("GodGame", "NoMiracleOrMiracleCooldown", "No miracle selected or is on cooldown."));
        return Result;
    }

    // Check for valid target.
    if(!Hit.bBlockingHit || !(Miracle->TargetingMode != EMiracleTargetingMode::Actor || IsValid(Hit.GetActor())))
    {
        Fail(EGodGameMiracleCastFailure::InvalidTarget, NSLOCTEXT("GodGame", "InvalidTarget", "Invalid miracle target."));
        return Result;
    }

    // Cast range check.
    if((Miracle->MaxCastRange > 0.0f) && (FVector::DistSquared(GetCastOrigin(), Hit.ImpactPoint) > FMath::Square(Miracle->MaxCastRange)))
    {
        Fail(EGodGameMiracleCastFailure::OutOfRange, NSLOCTEXT("GodGame", "OutOfRange", "Target is out of range."));
        return Result;
    }

    // Ensure the player has enough Influence to cast the Miracle.
    const UGodGameWorldSubsystem* pSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr;
    if(!pSubsystem || (pSubsystem->GetInfluence() < Miracle->InfluenceCost))
    {
        Fail(EGodGameMiracleCastFailure::InsufficientInfluence, NSLOCTEXT("GodGame", "NoInfluence", "Not enough influence."));
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

    // Ensure the player has enough Influence to cast the Miracle.
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
        // Align the spawn rotation to the surface normal.
        SpawnRotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();
    }

    // Spawn the miracle actor class.
    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = GetOwner() ? GetOwner()->GetInstigator() : nullptr;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if(IsValid(Miracle->MiracleActorClass))
    {
		SpawnedActor = pWorld->SpawnActor<AActor>(Miracle->MiracleActorClass, Hit.ImpactPoint, SpawnRotation, Params);
		if(!SpawnedActor)
		{
			Sim->AddInfluence(Miracle->InfluenceCost);
			OutResult.bCanCast = false;
			OutResult.FailureReason = EGodGameMiracleCastFailure::SpawnFailed;
			OutResult.Message = NSLOCTEXT("GodGame", "SpawnFailed", "Miracle effect failed to spawn.");
			return false;
		}
    }

	// Native prototype fallback: every miracle visibly pulses and improves nearby wellbeing/faith.
	const float Radius = FMath::Max(DefaultMiracleRadius, Miracle->EffectRadius);
	DrawDebugSphere(pWorld, Hit.ImpactPoint, Radius, 32, DefaultMiracleDebugDrawColor, false, 5.0f, 0, 5.0f);
	UGodGameBlueprintLibrary::ModifyNeedInRadius(this, Hit.ImpactPoint, Radius, EVillagerNeed::Hunger, DefaultMiracleHungerModifier, FGameplayTagContainer());
	UGodGameBlueprintLibrary::ModifyNeedInRadius(this, Hit.ImpactPoint, Radius, EVillagerNeed::Safety, DefaultMiracleSafetyModifier, FGameplayTagContainer());
	UGodGameBlueprintLibrary::ModifyFaithInRadius(this, Hit.ImpactPoint, Radius, DefaultMiracleFaithModifier, FGameplayTagContainer());

    // Set the last cast time for the miracle (to track cooldown).
    MiracleLastCastTimeMap.Add(Miracle, pWorld->GetTimeSeconds());
    
    // Broadcast the miracle cast event.
    OnMiracleCast.Broadcast(Miracle, SpawnedActor);
    return true;
}

// Attempts to cast the selected miracle at a cursor trace.
bool UGodGameMiracleComponent::CastSelectedMiracleFromHit(const FHitResult& Hit, AActor*& SpawnedActor, FGodGameMiracleCastCheck& ResultOut)
{
    return(CastMiracleFromHit(SelectedMiracle, Hit, SpawnedActor, ResultOut));
}

// Updates the selected miracle's preview while the player is aiming, creating it lazily when needed.
void UGodGameMiracleComponent::UpdatePreview(const FHitResult& Hit)
{
    if(!SelectedMiracle)
    {
        // Invalid state; should have the native fallback.
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
