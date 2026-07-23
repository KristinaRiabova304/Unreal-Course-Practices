// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMGameMode.h"
#include "TDMGameState.h"
#include "TDMPlayerState.h"
#include "TDMCharacter.h"
#include "TDMPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATDMGameMode::ATDMGameMode()
{
	// Unreal won't pick up our subclasses unless we register them here.
	GameStateClass        = ATDMGameState::StaticClass();
	PlayerStateClass      = ATDMPlayerState::StaticClass();
	PlayerControllerClass = ATDMPlayerController::StaticClass();
	DefaultPawnClass      = ATDMCharacter::StaticClass();

	// The scoreboard is UMG, created from the PlayerController — not an AHUD canvas.
	HUDClass = nullptr;

	// Start the match as soon as the world is ready (no lobby / ready-up step).
	bDelayedStart = false;
}

void ATDMGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ATDMPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ATDMPlayerState>() : nullptr)
	{
		PS->SetTeamId(ChooseTeamForNewPlayer());
	}
}

int32 ATDMGameMode::ChooseTeamForNewPlayer() const
{
	int32 Counts[ATDMGameState::NumTeams] = { 0 };

	if (GameState)
	{
		// PlayerArray is already replicated and maintained for us.
		for (const APlayerState* PS : GameState->PlayerArray)
		{
			if (const ATDMPlayerState* TPS = Cast<ATDMPlayerState>(PS))
			{
				if (TPS->TeamId >= 0 && TPS->TeamId < ATDMGameState::NumTeams)
				{
					Counts[TPS->TeamId]++;
				}
			}
		}
	}

	// Tie goes to team 0, so the very first player lands on team 0.
	return (Counts[0] <= Counts[1]) ? 0 : 1;
}

void ATDMGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (ATDMGameState* GS = GetGameState<ATDMGameState>())
	{
		GS->SetRemainingTime(MatchDuration);
	}

	// 1-second server timer drives the countdown (preferred over Tick).
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ATDMGameMode::TickMatchTimer, 1.0f, true);
}

void ATDMGameMode::TickMatchTimer()
{
	ATDMGameState* GS = GetGameState<ATDMGameState>();
	if (!GS)
	{
		return;
	}

	const float NewTime = FMath::Max(0.0f, GS->RemainingMatchTime - 1.0f);
	GS->SetRemainingTime(NewTime);

	if (NewTime <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		EndMatch();
	}
}

void ATDMGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	if (const ATDMGameState* GS = GetGameState<ATDMGameState>())
	{
		const int32 Winner = GS->GetWinningTeam();
		UE_LOG(LogTemp, Log, TEXT("[TDM] Match ended. %s (Team0 %d - Team1 %d)"),
			Winner < 0 ? TEXT("Draw") : (Winner == 0 ? TEXT("Team 0 wins") : TEXT("Team 1 wins")),
			GS->GetTeamScore(0), GS->GetTeamScore(1));
	}
}

void ATDMGameMode::ScoreKill(AController* VictimController, AController* KillerController)
{
	// Belt-and-braces: this class only exists on the server, but assert intent.
	if (!HasAuthority())
	{
		return;
	}

	ATDMPlayerState* VictimPS = VictimController ? VictimController->GetPlayerState<ATDMPlayerState>() : nullptr;
	ATDMPlayerState* KillerPS = KillerController ? KillerController->GetPlayerState<ATDMPlayerState>() : nullptr;

	if (VictimPS)
	{
		VictimPS->AddDeath();
	}

	// Credit a kill + team score only for a genuine kill by a different player
	// (suicide / environment death adds a death but no score).
	if (KillerPS && KillerPS != VictimPS)
	{
		KillerPS->AddKill();

		if (ATDMGameState* GS = GetGameState<ATDMGameState>())
		{
			GS->AddTeamScore(KillerPS->TeamId);
		}
	}

	RespawnController(VictimController);
}

void ATDMGameMode::RespawnController(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	// Remove the dead pawn immediately.
	if (APawn* OldPawn = Controller->GetPawn())
	{
		Controller->UnPossess();
		OldPawn->Destroy();
	}

	// Respawn after a short delay via RestartPlayer (picks a PlayerStart & spawns DefaultPawn).
	FTimerHandle RespawnHandle;
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(
		this, &ATDMGameMode::RestartPlayer, Controller);
	GetWorldTimerManager().SetTimer(RespawnHandle, RespawnDelegate, RespawnDelay, false);
}
