// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LagprojectCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);


USTRUCT(BlueprintType)
struct FLagSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	float Timestamp = 0.0f;

	UPROPERTY()
	FTransform HeadTransform;

	UPROPERTY()
	FTransform TorsoTransform;
};


UCLASS(config = Game) 
class ALagprojectCharacter : public ACharacter
{
	GENERATED_BODY()

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

public:
	/** Constructor */
	ALagprojectCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();



	bool GetInterpolatedTransforms(float TargetTimestamp, FTransform& OutHead, FTransform& OutTorso);

	UFUNCTION(BlueprintCallable, Category = "Lag Compensation")
	void FireWeapon();

protected:

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lag Compensation")
	class ALagRewindWeapon* MyWeapon;

	TArray<FLagSnapshot> HitboxHistory;

	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation")
	FName HeadBoneName = TEXT("head");

	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation")
	FName TorsoBoneName = TEXT("spine_03");

	float TimeSinceLastSnapshot = 0.0f;
	const float SnapshotInterval = 0.05f;
	const float MaxHistoryLength = 1.0f;

	void SaveHitboxSnapshot();


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Test")
	bool bAutoPatrol = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Test")
	float PatrolRadius = 300.0f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Test")
	float PatrolSpeed = 250.0f;

protected:
	void TickAutoPatrol(float DeltaTime);

	FVector PatrolCenter = FVector::ZeroVector;
	float PatrolAngle = 0.0f;
	bool bPatrolCenterInitialized = false;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};