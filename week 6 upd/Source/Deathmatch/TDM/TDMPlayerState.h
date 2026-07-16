// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TDMPlayerState.generated.h"

/** Fired on every machine (via RepNotify on clients, directly on the server) whenever this
 *  player's team / kills / deaths change. The scoreboard binds to this to refresh on event. */
DECLARE_MULTICAST_DELEGATE(FTDMPlayerStatsChanged);

/**
 *  Persistent per-player state for the team deathmatch.
 *  Lives on the PlayerController, so it survives pawn death & respawn automatically.
 *  All mutation happens on the server; values replicate down to every client.
 */
UCLASS()
class ATDMPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ATDMPlayerState();

	/** Team this player belongs to (0 or 1). -1 = unassigned. */
	UPROPERTY(ReplicatedUsing = OnRep_TeamId, BlueprintReadOnly, Category = "TDM")
	int32 TeamId = -1;

	/** Kills scored by this player. */
	UPROPERTY(ReplicatedUsing = OnRep_Stats, BlueprintReadOnly, Category = "TDM")
	int32 Kills = 0;

	/** Times this player has died. */
	UPROPERTY(ReplicatedUsing = OnRep_Stats, BlueprintReadOnly, Category = "TDM")
	int32 Deaths = 0;

	/** Broadcast whenever any of the stats above change (server + all clients). */
	FTDMPlayerStatsChanged OnPlayerStatsChanged;

	//~ Server-only mutators (no-op if called without authority)
	void SetTeamId(int32 NewTeamId);
	void AddKill();
	void AddDeath();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UFUNCTION()
	void OnRep_TeamId();

	UFUNCTION()
	void OnRep_Stats();
};
