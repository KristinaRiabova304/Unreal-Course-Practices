// Copyright Epic Games, Inc. All Rights Reserved.

#include "HardenedCharacterMovementComponent.h"
#include "AntiCheatLog.h"
#include "GameFramework/Character.h"

UHardenedCharacterMovementComponent::UHardenedCharacterMovementComponent()
{
}

void UHardenedCharacterMovementComponent::BeginTeleportGrace()
{
	TeleportGraceRemaining = FMath::Max(TeleportGraceRemaining, TeleportGraceSeconds);
}

bool UHardenedCharacterMovementComponent::ServerCheckClientError(
	float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation,
	const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	// Always let the engine's own error detection run first; we only *add* a rejection reason.
	bool bHasError = Super::ServerCheckClientError(
		ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation,
		ClientMovementBase, ClientBaseBoneName, ClientMovementMode);

	// Age the grace window using this move's delta.
	if (TeleportGraceRemaining > 0.0f)
	{
		TeleportGraceRemaining = FMath::Max(0.0f, TeleportGraceRemaining - DeltaTime);
	}

	// Conditions under which a large step is legitimate and must NOT be treated as cheating.
	const bool bLegitLargeMove =
		bJustTeleported ||
		MovementMode == MOVE_None ||
		HasAnimRootMotion() ||
		CurrentRootMotion.HasActiveRootMotionSources() ||
		TeleportGraceRemaining > 0.0f ||
		!bHasLastClientLocation ||
		DeltaTime <= 0.0f;

	if (!bHasError && !bLegitLargeMove)
	{
		// Horizontal displacement the client claims for this move.
		const float ClaimedDistance = FVector::Dist2D(ClientWorldLocation, LastClientLocation);

		// Budget = (max ground speed + any moving-platform contribution) x margin x dt, plus slack.
		float BaseSpeed = 0.0f;
		if (ClientMovementBase)
		{
			BaseSpeed = MovementBaseUtility::GetMovementBaseVelocity(ClientMovementBase, ClientBaseBoneName).Size2D();
		}

		const float AllowedSpeed = GetMaxSpeed() * SpeedSafetyMargin + BaseSpeed;
		const float AllowedDistance = AllowedSpeed * DeltaTime + DistanceEpsilon;

		if (ClaimedDistance > AllowedDistance)
		{
			const float ClaimedSpeed = ClaimedDistance / DeltaTime;
			LogAntiCheatRejection(
				this, GetCharacterOwner(), TEXT("Move"),
				FString::Printf(TEXT("impossible speed: claimed %.0f cm/s > allowed %.0f cm/s (dt=%.3f, moved %.1fcm)"),
					ClaimedSpeed, AllowedSpeed, DeltaTime, ClaimedDistance));

			// Prioritise the correction; returning true makes the engine send its standard,
			// prediction-aware ClientAdjustPosition (no manual teleport on our part).
			bNetworkLargeClientCorrection = true;
			bHasError = true;
		}
	}

	// Track state for the next move. After any correction, anchor to the server's authoritative
	// location (where the client will be snapped to) and skip a frame so the snap-back itself is
	// not misread as a second violation.
	if (bHasError)
	{
		BeginTeleportGrace();
		LastClientLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : ClientWorldLocation;
	}
	else
	{
		LastClientLocation = ClientWorldLocation;
	}
	bHasLastClientLocation = true;

	return bHasError;
}
