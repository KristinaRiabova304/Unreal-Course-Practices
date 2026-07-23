// Copyright Epic Games, Inc. All Rights Reserved.

#include "AntiCheatComponent.h"
#include "AntiCheatLog.h"
#include "Engine/World.h"

UAntiCheatComponent::UAntiCheatComponent()
{
	// Pure bookkeeping component — never needs to tick.
	PrimaryComponentTick.bCanEverTick = false;
}

float UAntiCheatComponent::GetServerTimeSeconds() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds();
	}
	return 0.0f;
}

bool UAntiCheatComponent::AllowAction(const FName ActionName, const float TokensPerSecond, const float BurstCapacity)
{
	FRpcRateLimit Limit;
	Limit.TokensPerSecond = TokensPerSecond;
	Limit.BurstCapacity = BurstCapacity;
	return AllowAction(ActionName, Limit);
}

bool UAntiCheatComponent::AllowAction(const FName ActionName, const FRpcRateLimit& Limit)
{
	// Rate limiting is a server-authoritative concern; clients never gate themselves.
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return true;
	}

	const float Now = GetServerTimeSeconds();

	FBucketState& Bucket = Buckets.FindOrAdd(ActionName);

	if (!Bucket.bInitialized)
	{
		// First call for this action starts with a full bucket so the initial burst is allowed.
		Bucket.Tokens = Limit.BurstCapacity;
		Bucket.LastRefillServerTime = Now;
		Bucket.bInitialized = true;
	}
	else
	{
		// Continuous refill, capped at burst capacity.
		const float Elapsed = FMath::Max(0.0f, Now - Bucket.LastRefillServerTime);
		Bucket.Tokens = FMath::Min(Limit.BurstCapacity, Bucket.Tokens + Elapsed * Limit.TokensPerSecond);
		Bucket.LastRefillServerTime = Now;
	}

	if (Bucket.Tokens >= 1.0f)
	{
		Bucket.Tokens -= 1.0f;
		return true;
	}

	// Out of budget: reject and log a structured record for later analysis.
	LogAntiCheatRejection(
		this, Owner, ActionName,
		FString::Printf(TEXT("rate limit exceeded (%.1f/s, burst %.0f)"), Limit.TokensPerSecond, Limit.BurstCapacity));

	return false;
}
