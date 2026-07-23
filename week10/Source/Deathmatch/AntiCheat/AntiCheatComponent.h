// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AntiCheatComponent.generated.h"

/**
 *  Configurable limit for a single rate-limited action.
 *
 *  A token bucket (rather than a hard "min interval") is used so that legitimate packet
 *  bunching and brief lag spikes do not punish honest players: tokens refill continuously at
 *  TokensPerSecond and the bucket can hold up to BurstCapacity, absorbing short bursts while
 *  still bounding the long-run rate.
 */
USTRUCT(BlueprintType)
struct FRpcRateLimit
{
	GENERATED_BODY()

	/** Sustained allowed calls per second (bucket refill rate). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AntiCheat", meta = (ClampMin = "0.1"))
	float TokensPerSecond = 8.0f;

	/** Maximum tokens the bucket can hold — the size of a tolerated burst. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AntiCheat", meta = (ClampMin = "1.0"))
	float BurstCapacity = 12.0f;
};

/**
 *  Reusable, server-side RPC rate limiter.
 *
 *  Ownership: attached to the APlayerController, so state is inherently per-player (one
 *  component instance per player) and survives pawn death/respawn. Buckets are keyed by
 *  action name, so limiting is per-player AND per-action — never a single global counter.
 *
 *  The component only does work on the server (HasAuthority()); calling AllowAction() on a
 *  client always returns true so shared client/server code paths stay simple.
 */
UCLASS(ClassGroup = (AntiCheat), meta = (BlueprintSpawnableComponent))
class UAntiCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAntiCheatComponent();

	/**
	 *  Consume one token for the named action.
	 *  @return true if the call is within budget (token consumed); false if it must be rejected.
	 *  On rejection a structured record is written to LogAntiCheat automatically.
	 */
	bool AllowAction(const FName ActionName, const FRpcRateLimit& Limit);

	/** Convenience overload taking explicit numbers instead of a struct. */
	bool AllowAction(const FName ActionName, const float TokensPerSecond, const float BurstCapacity);

private:

	/** Mutable per-action bucket state. */
	struct FBucketState
	{
		float Tokens = 0.0f;
		float LastRefillServerTime = 0.0f;
		bool bInitialized = false;
	};

	/** One bucket per action name for the owning player. */
	TMap<FName, FBucketState> Buckets;

	/** Current authoritative server time in seconds, or 0 if unavailable. */
	float GetServerTimeSeconds() const;
};
