#include "TDMGameMode.h"
#include "TDMGameState.h"
#include "TDMPlayerState.h"
#include "TimerManager.h" 
#include "UObject/ConstructorHelpers.h"

ATDMGameMode::ATDMGameMode()
{
  
    GameStateClass = ATDMGameState::StaticClass();
    PlayerStateClass = ATDMPlayerState::StaticClass();

  
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}

void ATDMGameMode::BeginPlay()
{
    Super::BeginPlay();

    ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();
    if (TDMGameState)
    {
        TDMGameState->RemainingMatchTime = MatchDuration;

   
        GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ATDMGameMode::AdvanceTimer, 1.0f, true);
    }
}

void ATDMGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer) return;

    ATDMPlayerState* NewPS = NewPlayer->GetPlayerState<ATDMPlayerState>();
    ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();

    if (NewPS && TDMGameState)
    {
        int32 Team0Count = 0;
        int32 Team1Count = 0;

       
        for (APlayerState* PS : TDMGameState->PlayerArray)
        {
            ATDMPlayerState* TDMPS = Cast<ATDMPlayerState>(PS);
            if (TDMPS && TDMPS != NewPS) 
            {
                if (TDMPS->TeamId == 0) Team0Count++;
                else if (TDMPS->TeamId == 1) Team1Count++;
            }
        }

        NewPS->TeamId = (Team0Count <= Team1Count) ? 0 : 1;
    }
}

void ATDMGameMode::ScoreKill(AController* VictimController, AController* KillerController)
{
    ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();
    if (!TDMGameState) return;

    
    if (VictimController)
    {
        ATDMPlayerState* VictimPS = VictimController->GetPlayerState<ATDMPlayerState>();
        if (VictimPS) VictimPS->Deaths++;
    }

   
    if (KillerController && KillerController != VictimController)
    {
        ATDMPlayerState* KillerPS = KillerController->GetPlayerState<ATDMPlayerState>();
        if (KillerPS)
        {
            KillerPS->Kills++;

            if (TDMGameState->TeamScores.IsValidIndex(KillerPS->TeamId))
            {
                TDMGameState->TeamScores[KillerPS->TeamId]++;
            }
        }
    }
}

void ATDMGameMode::AdvanceTimer()
{
    ATDMGameState* TDMGameState = GetGameState<ATDMGameState>();
    if (TDMGameState)
    {
        TDMGameState->RemainingMatchTime--;

        if (TDMGameState->RemainingMatchTime <= 0.0f)
        {
            GetWorldTimerManager().ClearTimer(MatchTimerHandle);
            EndMatch();
        }
    }
}