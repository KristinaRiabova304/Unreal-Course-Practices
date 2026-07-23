// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMPlayerController.h"
#include "TDMScoreboardWidget.h"
#include "TDMCharacter.h"
#include "TDMPlayerState.h"
#include "AntiCheatComponent.h"
#include "AntiCheatLog.h"
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

	// Per-player rate limiter. Lives on the controller, so its buckets are naturally per player.
	AntiCheat = CreateDefaultSubobject<UAntiCheatComponent>(TEXT("AntiCheat"));

	// Fire: sustained 8/s with a 12-shot burst. Reload: 1/s with a 2-request burst.
	FireRateLimit.TokensPerSecond = 8.0f;
	FireRateLimit.BurstCapacity = 12.0f;
	ReloadRateLimit.TokensPerSecond = 1.0f;
	ReloadRateLimit.BurstCapacity = 2.0f;
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

	// Bind fire (left mouse) and reload (R) on the controller's (legacy) input component.
	// These keys aren't mapped in the Enhanced Input contexts, so they coexist with movement.
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATDMPlayerController::OnFirePressed);
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ATDMPlayerController::OnReloadPressed);
	}
}

void ATDMPlayerController::OnFirePressed()
{
	// Send the client's own view as the *claimed* aim. The server validates it against its own
	// authoritative view before trusting anything, and always traces from the server view point.
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);

	ServerFire(ViewLocation, ViewRotation.Vector());
}

void ATDMPlayerController::OnReloadPressed()
{
	ServerReload();
}

void ATDMPlayerController::ServerFire_Implementation(FVector ClientAimOrigin, FVector ClientAimDir)
{
	// ---- 1. Rate limit: reject fire spam / auto-fire macros before any work. --------------
	if (!AntiCheat || !AntiCheat->AllowAction(TEXT("Fire"), FireRateLimit))
	{
		return;
	}

	// ---- 2. Shooter must exist and be alive. ----------------------------------------------
	ATDMCharacter* MyPawn = GetPawn<ATDMCharacter>();
	if (!MyPawn || MyPawn->IsDead())
	{
		LogAntiCheatRejection(this, this, TEXT("Fire"), TEXT("no living pawn (dead man's fire)"));
		return;
	}

	// ---- 3. Ammo state must permit a shot (invalid ammo/state transition). ----------------
	if (!MyPawn->HasAmmo())
	{
		LogAntiCheatRejection(this, this, TEXT("Fire"), TEXT("empty magazine"));
		return;
	}

	// Server's authoritative view point — the only geometry we trust for the actual hitscan.
	FVector ServerViewLocation;
	FRotator ServerViewRotation;
	GetPlayerViewPoint(ServerViewLocation, ServerViewRotation);
	const FVector ServerViewDir = ServerViewRotation.Vector();

	// ---- 4. Impossible target claim: reject a muzzle origin far from the server pawn. ------
	if (FVector::Dist(ClientAimOrigin, ServerViewLocation) > MaxAimOriginError)
	{
		LogAntiCheatRejection(this, this, TEXT("Fire"),
			FString::Printf(TEXT("aim origin mismatch: %.0fcm from server view (max %.0f)"),
				FVector::Dist(ClientAimOrigin, ServerViewLocation), MaxAimOriginError));
		return;
	}

	// ---- 5. Impossible target claim: reject aim snapped far from the server view direction. -
	if (!ClientAimDir.Normalize())
	{
		LogAntiCheatRejection(this, this, TEXT("Fire"), TEXT("degenerate aim direction"));
		return;
	}
	const float CosLimit = FMath::Cos(FMath::DegreesToRadians(MaxAimDeviationDegrees));
	if (FVector::DotProduct(ServerViewDir, ClientAimDir) < CosLimit)
	{
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ServerViewDir, ClientAimDir)));
		LogAntiCheatRejection(this, this, TEXT("Fire"),
			FString::Printf(TEXT("aim deviation %.1f deg exceeds max %.1f"), Angle, MaxAimDeviationDegrees));
		return;
	}

	// All input validated — commit the ammo state change, then run the authoritative hitscan.
	MyPawn->ConsumeAmmo();

	const FVector TraceEnd = ServerViewLocation + ServerViewDir * FireRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(MyPawn);
	Params.bTraceComplex = false;

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, ServerViewLocation, TraceEnd, ECC_Pawn, Params))
	{
		return; // Clean miss — server geometry already guarantees line of sight & range.
	}

	ATDMCharacter* Victim = Cast<ATDMCharacter>(Hit.GetActor());
	if (!Victim)
	{
		return;
	}

	// ---- 6. Excessive distance guard (defence in depth; the trace already bounds this). ----
	if (Hit.Distance > FireRange)
	{
		LogAntiCheatRejection(this, this, TEXT("Fire"),
			FString::Printf(TEXT("target beyond range: %.0fcm > %.0f"), Hit.Distance, FireRange));
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
		Victim, FireDamage, ServerViewDir, Hit, this, MyPawn, UDamageType::StaticClass());
}

void ATDMPlayerController::ServerReload_Implementation()
{
	// Rate limit reload requests (spamming reload can be used to cancel animations / desync state).
	if (!AntiCheat || !AntiCheat->AllowAction(TEXT("Reload"), ReloadRateLimit))
	{
		return;
	}

	ATDMCharacter* MyPawn = GetPawn<ATDMCharacter>();
	if (!MyPawn || MyPawn->IsDead())
	{
		LogAntiCheatRejection(this, this, TEXT("Reload"), TEXT("no living pawn"));
		return;
	}

	// Invalid state transition: reloading an already-full magazine is a no-op we refuse to honour.
	if (MyPawn->IsMagazineFull())
	{
		LogAntiCheatRejection(this, this, TEXT("Reload"), TEXT("magazine already full"));
		return;
	}

	MyPawn->RefillMagazine();
}
