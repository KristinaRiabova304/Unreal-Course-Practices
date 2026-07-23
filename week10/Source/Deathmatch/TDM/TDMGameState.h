// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TDMGameState.generated.h"

/** Fired when the replicated team scores change (server + all clients). */
DECLARE_MULTICAST_DELEGATE(FTDMScoresChanged);

/** Fired when the replicated remaining match time changes (server + all clients). */
DECLARE_MULTICAST_DELEGATE(FTDMTimeChanged);

/**
 *  Replicated shared match state: per-team scores and the countdown timer.
 *  Written on the server, read everywhere. The scoreboard binds to the delegates below.
 */
UCLASS()
class ATDMGameState : public AGameState
{
	GENERATED_BODY()

public:

	ATDMGameState();

	/** Number of teams in the match. */
	static constexpr int32 NumTeams = 2;

	/** Score per team. Sized to NumTeams in BeginPlay (TArray has no size-based constructor). */
	UPROPERTY(ReplicatedUsing = OnRep_TeamScores, BlueprintReadOnly, Category = "TDM")
	TArray<int32> TeamScores;

	/** Seconds left in the match. Counted down on the server, replicated to clients. */
	UPROPERTY(ReplicatedUsing = OnRep_RemainingMatchTime, BlueprintReadOnly, Category = "TDM")
	float RemainingMatchTime = 0.0f;

	/** Delegates the UI binds to so it updates on replication events rather than on Tick. */
	FTDMScoresChanged OnScoresChanged;
	FTDMTimeChanged OnTimeChanged;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ Server-only mutators
	void AddTeamScore(int32 TeamId, int32 Amount = 1);
	void SetRemainingTime(float NewTime);

	/** Safe read (returns 0 for an out-of-range team). */
	int32 GetTeamScore(int32 TeamId) const;

	/** Index of the leading team, or -1 on a tie. */
	int32 GetWinningTeam() const;

protected:

	UFUNCTION()
	void OnRep_TeamScores();

	UFUNCTION()
	void OnRep_RemainingMatchTime();
};
