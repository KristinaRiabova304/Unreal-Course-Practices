// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMPlayerController.h"
#include "TDMScoreboardWidget.h"
#include "TDMCharacter.h"
#include "TDMPlayerState.h"
#include "InputMappingContext.h"
#include "Components/InputComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "UObject/ConstructorHelpers.h"

ATDMPlayerController::ATDMPlayerController()
{
	// Load the template input mapping contexts so movement/look work when this C++
	// controller is used directly (they are normally assigned on the BP controller).
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC(TEXT("/Game/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookIMC(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	if (DefaultIMC.Succeeded())
	{
		DefaultMappingContexts.AddUnique(DefaultIMC.Object);
	}
	if (MouseLookIMC.Succeeded())
	{
		DefaultMappingContexts.AddUnique(MouseLookIMC.Object);
	}
}

void ATDMPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	// Only the owning client needs the scoreboard. AddToViewport() + bind-on-event
	// happens inside the widget; late joiners just get replicated state as it arrives.
	if (IsLocalController() && !ScoreboardWidget)
	{
		TSubclassOf<UTDMScoreboardWidget> WidgetClass = ScoreboardWidgetClass;
		if (!WidgetClass)
		{
			WidgetClass = UTDMScoreboardWidget::StaticClass();
		}

		ScoreboardWidget = CreateWidget<UTDMScoreboardWidget>(this, WidgetClass);
		if (ScoreboardWidget)
		{
			ScoreboardWidget->AddToViewport();
		}
	}
}

void ATDMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind fire on the controller's (legacy) input component. Left mouse isn't mapped in
	// the Enhanced Input contexts, so this coexists with the character's movement bindings.
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATDMPlayerController::OnFirePressed);
	}
}

void ATDMPlayerController::OnFirePressed()
{
	// Route to the server, which owns the authoritative hitscan & damage.
	ServerFire();
}

void ATDMPlayerController::ServerFire_Implementation()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * FireRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(MyPawn);
	Params.bTraceComplex = false;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Pawn, Params))
	{
		ATDMCharacter* Victim = Cast<ATDMCharacter>(Hit.GetActor());
		if (!Victim)
		{
			return;
		}

		// No friendly fire: ignore hits on a player from our own team.
		const ATDMPlayerState* MyPS = GetPlayerState<ATDMPlayerState>();
		const ATDMPlayerState* VictimPS = Victim->GetController() ? Victim->GetController()->GetPlayerState<ATDMPlayerState>() : nullptr;
		if (MyPS && VictimPS && MyPS->TeamId == VictimPS->TeamId)
		{
			return;
		}

		// Standard engine damage: TDMCharacter::TakeDamage handles death & scoring.
		UGameplayStatics::ApplyPointDamage(
			Victim, FireDamage, ViewRotation.Vector(), Hit, this, MyPawn, UDamageType::StaticClass());
	}
}
