// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TDMGameMode.generated.h"

/**
 *  Server-authoritative match flow for the team deathmatch.
 *
 *  AGameMode (not AGameModeBase) gives us the WaitingToStart / InProgress / WaitingPostMatch
 *  state machine plus EndMatch() — exactly what a fixed-length countdown match needs.
 *
 *  The GameMode exists only on the server, so every method here runs with authority.
 */
UCLASS()
class ATDMGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ATDMGameMode();

	/** Assigns each joining player to the team with fewer members. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/**
	 *  Applies the outcome of a kill: +1 death to the victim, and (for a real enemy kill)
	 *  +1 kill to the killer and +1 to the killer's team score. Then respawns the victim.
	 *  Server only.
	 */
	void ScoreKill(AController* VictimController, AController* KillerController);

	/** Match length in seconds. */
	UPROPERTY(EditDefaultsOnly, Category = "TDM")
	float MatchDuration = 300.0f;

	/** Delay before a dead player respawns. */
	UPROPERTY(EditDefaultsOnly, Category = "TDM")
	float RespawnDelay = 3.0f;

protected:

	//~ AGameMode match-state hooks
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	/** Called once per second while the match is in progress; ends the match at 0. */
	void TickMatchTimer();

	/** Destroys the dead pawn and schedules a fresh spawn. */
	void RespawnController(AController* Controller);

	/** Returns the team index (0/1) with the fewest currently-assigned players. */
	int32 ChooseTeamForNewPlayer() const;

	/** Drives the countdown. */
	FTimerHandle MatchTimerHandle;
};
