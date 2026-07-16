// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMCharacter.h"
#include "TDMGameMode.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ATDMCharacter::ATDMCharacter()
{
	// Give the C++ default pawn the standard mannequin mesh + anim so it is visible in PIE
	// without needing a Blueprint. (The template normally sets these on the BP asset.)
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (MeshAsset.Succeeded() && GetMesh())
	{
		GetMesh()->SetSkeletalMeshAsset(MeshAsset.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimBP.Succeeded() && GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	}

	// Populate the Enhanced Input actions declared on the base character (also normally set on
	// the BP), so movement/jump work when this C++ class is used directly as the default pawn.
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpIA(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveIA(TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookIA(TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookIA(TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	if (JumpIA.Succeeded())     { JumpAction = JumpIA.Object; }
	if (MoveIA.Succeeded())     { MoveAction = MoveIA.Object; }
	if (LookIA.Succeeded())     { LookAction = LookIA.Object; }
	if (MouseLookIA.Succeeded()) { MouseLookAction = MouseLookIA.Object; }
}

void ATDMCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
}

void ATDMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMCharacter, Health);
}

float ATDMCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Damage is only authoritative on the server.
	if (!HasAuthority() || bIsDead || ActualDamage <= 0.0f)
	{
		return ActualDamage;
	}

	Health = FMath::Max(0.0f, Health - ActualDamage);

	if (Health <= 0.0f)
	{
		Die(EventInstigator);
	}

	return ActualDamage;
}

void ATDMCharacter::Die(AController* Killer)
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	AController* VictimController = GetController();

	// The GameMode owns match/scoring authority and the respawn flow.
	if (ATDMGameMode* GameMode = GetWorld()->GetAuthGameMode<ATDMGameMode>())
	{
		GameMode->ScoreKill(VictimController, Killer);
	}
}
