// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMPlayerState.h"
#include "Net/UnrealNetwork.h"

ATDMPlayerState::ATDMPlayerState()
{
}

void ATDMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// PlayerState replicates per-property. Forgetting these would leave the values
	// silently default-initialized on the clients.
	DOREPLIFETIME(ATDMPlayerState, TeamId);
	DOREPLIFETIME(ATDMPlayerState, Kills);
	DOREPLIFETIME(ATDMPlayerState, Deaths);
}

void ATDMPlayerState::SetTeamId(int32 NewTeamId)
{
	if (!HasAuthority())
	{
		return;
	}

	TeamId = NewTeamId;
	// The server never receives its own RepNotify, so broadcast locally too
	// (matters for a listen-server host player).
	OnPlayerStatsChanged.Broadcast();
}

void ATDMPlayerState::AddKill()
{
	if (!HasAuthority())
	{
		return;
	}

	++Kills;
	OnPlayerStatsChanged.Broadcast();
}

void ATDMPlayerState::AddDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	++Deaths;
	OnPlayerStatsChanged.Broadcast();
}

void ATDMPlayerState::OnRep_TeamId()
{
	OnPlayerStatsChanged.Broadcast();
}

void ATDMPlayerState::OnRep_Stats()
{
	OnPlayerStatsChanged.Broadcast();
}
