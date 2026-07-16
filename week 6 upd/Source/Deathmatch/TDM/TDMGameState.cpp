// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMGameState.h"
#include "Net/UnrealNetwork.h"

ATDMGameState::ATDMGameState()
{
}

void ATDMGameState::BeginPlay()
{
	Super::BeginPlay();

	// TArray<int32> has no size-based constructor. Writing TeamScores[i] before this
	// would assert with an index-out-of-bounds. Runs on server and clients so the array
	// is always valid, even before the first replication update arrives.
	if (TeamScores.Num() < NumTeams)
	{
		TeamScores.SetNum(NumTeams);
	}
}

void ATDMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameState, TeamScores);
	DOREPLIFETIME(ATDMGameState, RemainingMatchTime);
}

void ATDMGameState::AddTeamScore(int32 TeamId, int32 Amount)
{
	if (!HasAuthority() || !TeamScores.IsValidIndex(TeamId))
	{
		return;
	}

	TeamScores[TeamId] += Amount;
	OnScoresChanged.Broadcast();
}

void ATDMGameState::SetRemainingTime(float NewTime)
{
	if (!HasAuthority())
	{
		return;
	}

	RemainingMatchTime = NewTime;
	OnTimeChanged.Broadcast();
}

int32 ATDMGameState::GetTeamScore(int32 TeamId) const
{
	return TeamScores.IsValidIndex(TeamId) ? TeamScores[TeamId] : 0;
}

int32 ATDMGameState::GetWinningTeam() const
{
	if (TeamScores.Num() < NumTeams)
	{
		return -1;
	}

	if (TeamScores[0] == TeamScores[1])
	{
		return -1; // tie
	}

	return TeamScores[0] > TeamScores[1] ? 0 : 1;
}

void ATDMGameState::OnRep_TeamScores()
{
	OnScoresChanged.Broadcast();
}

void ATDMGameState::OnRep_RemainingMatchTime()
{
	OnTimeChanged.Broadcast();
}
