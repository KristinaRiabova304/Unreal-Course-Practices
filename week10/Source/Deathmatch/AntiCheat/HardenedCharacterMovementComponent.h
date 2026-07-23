// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HardenedCharacterMovementComponent.generated.h"

/**
 *  CharacterMovementComponent that adds a server-authoritative speed check on top of the
 *  engine's normal client-error detection.
 *
 *  We extend the authoritative path (ServerCheckClientError) rather than re-implementing
 *  movement: when the client's claimed position for a move implies a horizontal speed above
 *  MaxSpeed x SafetyMargin, we flag the move as an error. The engine then issues its standard
 *  ClientAdjustPosition correction, which is prediction-aware — so we never manually teleport
 *  the pawn and never fight client-side prediction.
 *
 *  Legitimate non-walking motion is explicitly allowed: freshly spawned/respawned pawns, engine
 *  teleports (bJustTeleported), a short post-teleport grace window, active root motion, and
 *  motion imparted by a moving platform (the base velocity is added to the budget).
 */
UCLASS()
class UHardenedCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UHardenedCharacterMovementComponent();

	/**
	 *  Open a grace window during which the speed check is skipped. Call this around any
	 *  intentional, non-simulated relocation (teleport ability, level transition seam, etc.).
	 *  Spawn/respawn is already covered because a fresh component has no prior sample.
	 */
	void BeginTeleportGrace();

	//~ Begin UCharacterMovementComponent interface
	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	//~ End UCharacterMovementComponent interface

	/** Multiplier applied to MaxSpeed before rejecting; headroom for latency and step-up. */
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat", meta = (ClampMin = "1.0"))
	float SpeedSafetyMargin = 1.5f;

	/** Seconds after a flagged teleport during which the speed check is suppressed. */
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat", meta = (ClampMin = "0.0"))
	float TeleportGraceSeconds = 0.5f;

	/** Small absolute slack (cm) added to the per-move budget to absorb float noise. */
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat", meta = (ClampMin = "0.0"))
	float DistanceEpsilon = 2.0f;

private:

	/** Last accepted client-claimed world location, used to measure per-move displacement. */
	FVector LastClientLocation = FVector::ZeroVector;
	bool bHasLastClientLocation = false;

	/** Remaining teleport-grace time (server seconds). */
	float TeleportGraceRemaining = 0.0f;
};
