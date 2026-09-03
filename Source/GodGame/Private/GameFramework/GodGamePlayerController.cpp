// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "GameFramework/GodGamePlayerController.h"

#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"

#include "JoyCoreNativeGameplayTags.h"
#include "Systems/Traits/ISystemicTraitProvider.h"

#include "Data/GodGameMiracleDefinition.h"
#include "Interaction/GodGameMiracleComponent.h"
#include "Simulation/GodGameWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGamePlayerController)

// Constructor.
AGodGamePlayerController::AGodGamePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    DefaultMouseCursor = EMouseCursor::Default;

	// Create the miracle component.
	MiracleComponent = CreateDefaultSubobject<UGodGameMiracleComponent>(TEXT("MiracleComponent"));
	
	// Prototype native fallback.
	RainMiracleClass = TSoftClassPtr<UGodGameMiracleDefinition>(FSoftObjectPath(TEXT("/GodGame/Data/Miracles/Rain_Miracle.Rain_Miracle_C")));
}

// Begin Play.
void AGodGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	SetInputMode(InputMode);
	SelectRainMiracle();
}

// Input setup.
//	TODO (trent, 8/26/26): Hacky, expose to the editor.
void AGodGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::HandlePrimaryAction);
		InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ThisClass::HandleSecondaryAction);
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::SelectRainMiracle);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::ClearSelection);
	}
}

// Player tick.
void AGodGamePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FHitResult Hit;
	if(TraceGodCursor(Hit))
	{
		MiracleComponent->UpdatePreview(Hit);
	}
	else
	{
		MiracleComponent->HidePreview();
	}
	
	RefreshPrototypeHUD();
}

void AGodGamePlayerController::HandlePrimaryAction()
{
	FHitResult Hit;
	if(!GEngine || !TraceGodCursor(Hit))
	{
		return;
	}

	// Cast the currently selected miracle.
	AActor* SpawnedActor = nullptr;
	FGodGameMiracleCastCheck Result;
	if(!MiracleComponent->CastSelectedMiracleFromHit(Hit, SpawnedActor, Result))
	{
		// Print debug message.
		GEngine->AddOnScreenDebugMessage(7002, 5.0f, FColor::Red, Result.Message.ToString());
	}
}

// Secondary action; Select actor under cursor.
void AGodGamePlayerController::HandleSecondaryAction()
{
	SelectActorUnderCursor();
}

void AGodGamePlayerController::SelectRainMiracle()
{
	if(UClass* pMiracleClass = RainMiracleClass.LoadSynchronous())
	{
		MiracleComponent->SelectMiracle(pMiracleClass->GetDefaultObject<UGodGameMiracleDefinition>());
	}
}

void AGodGamePlayerController::RefreshPrototypeHUD() const
{
	if(!bShowPrototypeHUD || !GEngine)
	{
		return;
	}

	const UGodGameWorldSubsystem* pSimulation = GetWorld() ? GetWorld()->GetSubsystem<UGodGameWorldSubsystem>() : nullptr;
	const float Influence = pSimulation ? pSimulation->GetInfluence() : 0.0f;
	const int32 Believers = pSimulation ? pSimulation->BelieverCount : 0;
	FString MiracleName = MiracleComponent && MiracleComponent->SelectedMiracle ? (MiracleComponent->SelectedMiracle->DisplayName.ToString()) : TEXT("None");
	if(MiracleName.IsEmpty() && MiracleComponent && MiracleComponent->SelectedMiracle)
	{
		MiracleName = MiracleComponent->SelectedMiracle->GetName();
	}

	// Debug HUD.
	const FString Status = FString::Printf(TEXT("GOD GAME  |  Influence %.0f  |  Believers %d  |  Miracle: %s\nLMB cast  |  RMB select  |  1 rain  |  Esc clear"), Influence, Believers, *MiracleName);
	GEngine->AddOnScreenDebugMessage(7001, 0.05f, FColor(120, 220, 255), Status);
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
    const FVector EndPoint = WorldDirection*Distance + WorldOrigin;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(GodCursorTrace), true);
    if(const APawn* pPawn = GetPawn())
    {
        Params.AddIgnoredActor(pPawn);
    }

    return(pWorld->LineTraceSingleByChannel(HitOut, WorldOrigin, EndPoint, CursorTraceChannel, Params));
}

// Selects the actor under the cursor unless its God Game tag component opts out of selection.
AActor* AGodGamePlayerController::SelectActorUnderCursor()
{
    FHitResult Hit = FHitResult();
    AActor* pCandidate = TraceGodCursor(Hit) ? Hit.GetActor() : nullptr;

    // A tag component is optional, but if present it can explicitly opt an actor out of god selection.
    if(IsValid(pCandidate))
    {
        if(const ISystemicTraitProvider* pTraitProvider = Cast<ISystemicTraitProvider>(pCandidate->FindComponentByInterface(USystemicTraitProvider::StaticClass())))
        {
            if(!pTraitProvider->HasTrait(TAG_System_Trait_Selectable))
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
