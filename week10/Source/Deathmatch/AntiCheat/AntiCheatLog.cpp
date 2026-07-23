// Copyright Epic Games, Inc. All Rights Reserved.

#include "AntiCheatLog.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogAntiCheat);

namespace
{
	/** Resolve a stable "name (id)" identity from a Controller / Pawn / PlayerState. */
	FString ResolvePlayerIdentity(const AActor* Offender)
	{
		const APlayerState* PS = nullptr;

		if (const APlayerState* AsPS = Cast<APlayerState>(Offender))
		{
			PS = AsPS;
		}
		else if (const AController* AsController = Cast<AController>(Offender))
		{
			PS = AsController->PlayerState;
		}
		else if (const APawn* AsPawn = Cast<APawn>(Offender))
		{
			PS = AsPawn->GetPlayerState();
		}

		if (PS)
		{
			return FString::Printf(TEXT("%s (Id=%d)"), *PS->GetPlayerName(), PS->GetPlayerId());
		}

		return Offender ? Offender->GetName() : TEXT("<unknown>");
	}
}

void LogAntiCheatRejection(const UObject* WorldContext, const AActor* Offender, const FName Action, const FString& Reason)
{
	// Authoritative server time (seconds since world start) is the most useful field for
	// correlating rejections with gameplay; pair it with a UTC wall-clock stamp for humans.
	float ServerSeconds = 0.0f;
	if (WorldContext)
	{
		if (const UWorld* World = WorldContext->GetWorld())
		{
			ServerSeconds = World->GetTimeSeconds();
		}
	}

	const FString Timestamp = FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d %H:%M:%S"));

	UE_LOG(LogAntiCheat, Warning,
		TEXT("[REJECT] %sZ (t=%.2fs) | player=%s | action=%s | reason=%s"),
		*Timestamp, ServerSeconds, *ResolvePlayerIdentity(Offender), *Action.ToString(), *Reason);
}
