// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagprojectCharacter.h"
#include "LagRewindWeapon.h" 
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Lagproject.h"

ALagprojectCharacter::ALagprojectCharacter()
{
	PrimaryActorTick.bCanEverTick = true; 
	MyWeapon = nullptr;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ALagprojectCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	if (HasAuthority())
	{
		PatrolCenter = GetActorLocation();
		bPatrolCenterInitialized = true;
	}
}

void ALagprojectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (HasAuthority() && GetWorld() && GetWorld()->GetGameState())
	{
		TimeSinceLastSnapshot += DeltaTime;
		if (TimeSinceLastSnapshot >= SnapshotInterval)
		{
			SaveHitboxSnapshot();
			TimeSinceLastSnapshot = 0.0f;
		}
	}


	if (HasAuthority() && bAutoPatrol)
	{
		TickAutoPatrol(DeltaTime);
	}
}

void ALagprojectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALagprojectCharacter, MyWeapon);
}

void ALagprojectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALagprojectCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ALagprojectCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALagprojectCharacter::Look);
	}
	else
	{
		UE_LOG(LogLagproject, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void ALagprojectCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ALagprojectCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ALagprojectCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ALagprojectCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ALagprojectCharacter::DoJumpStart() { Jump(); }
void ALagprojectCharacter::DoJumpEnd() { StopJumping(); }



void ALagprojectCharacter::SaveHitboxSnapshot()
{
	if (!GetMesh()) return;

	FLagSnapshot NewSnapshot;
	NewSnapshot.Timestamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	NewSnapshot.HeadTransform = GetMesh()->GetSocketTransform(HeadBoneName, RTS_World);
	NewSnapshot.TorsoTransform = GetMesh()->GetSocketTransform(TorsoBoneName, RTS_World);

	HitboxHistory.Add(NewSnapshot);

	float CurrentServerTime = NewSnapshot.Timestamp;
	while (HitboxHistory.Num() > 0 && (CurrentServerTime - HitboxHistory[0].Timestamp) > MaxHistoryLength)
	{
		HitboxHistory.RemoveAt(0);
	}
}

bool ALagprojectCharacter::GetInterpolatedTransforms(float TargetTimestamp, FTransform& OutHead, FTransform& OutTorso)
{
	if (HitboxHistory.Num() == 0) return false;

	if (TargetTimestamp <= HitboxHistory[0].Timestamp)
	{
		OutHead = HitboxHistory[0].HeadTransform;
		OutTorso = HitboxHistory[0].TorsoTransform;
		return true;
	}
	if (TargetTimestamp >= HitboxHistory.Last().Timestamp)
	{
		OutHead = HitboxHistory.Last().HeadTransform;
		OutTorso = HitboxHistory.Last().TorsoTransform;
		return true;
	}

	for (int32 i = 0; i < HitboxHistory.Num() - 1; ++i)
	{
		if (TargetTimestamp >= HitboxHistory[i].Timestamp && TargetTimestamp <= HitboxHistory[i + 1].Timestamp)
		{
			float Alpha = (TargetTimestamp - HitboxHistory[i].Timestamp) / (HitboxHistory[i + 1].Timestamp - HitboxHistory[i].Timestamp);

			OutHead.Blend(HitboxHistory[i].HeadTransform, HitboxHistory[i + 1].HeadTransform, Alpha);
			OutTorso.Blend(HitboxHistory[i].TorsoTransform, HitboxHistory[i + 1].TorsoTransform, Alpha);
			return true;
		}
	}
	return false;
}

void ALagprojectCharacter::FireWeapon()
{
	if (!MyWeapon) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);

	FVector AimDir = CameraRotation.Vector().GetSafeNormal();

	if (GetWorld() && GetWorld()->GetGameState())
	{
		float CurrentTimestamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		MyWeapon->ServerFireWeapon(CameraLocation, AimDir, CurrentTimestamp);
	}
}



void ALagprojectCharacter::TickAutoPatrol(float DeltaTime)
{
	if (!bPatrolCenterInitialized || PatrolRadius <= 0.0f)
	{
		return;
	}

	const float AngularSpeed = PatrolSpeed / PatrolRadius;
	PatrolAngle += AngularSpeed * DeltaTime;


	if (PatrolAngle > 2.0f * PI)
	{
		PatrolAngle -= 2.0f * PI;
	}


	const FVector TangentDirection = FVector(-FMath::Sin(PatrolAngle), FMath::Cos(PatrolAngle), 0.0f);

	AddMovementInput(TangentDirection, 1.0f);


	if (!TangentDirection.IsNearlyZero())
	{
		const FRotator TargetRot = TangentDirection.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 8.0f));
	}
}