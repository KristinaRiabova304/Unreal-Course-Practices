#include "TDMPlayerState.h"
#include "Net/UnrealNetwork.h" 

ATDMPlayerState::ATDMPlayerState()
{
    TeamId = -1; 
    Kills = 0;
    Deaths = 0;
}

void ATDMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    
    DOREPLIFETIME(ATDMPlayerState, TeamId);
    DOREPLIFETIME(ATDMPlayerState, Kills);
    DOREPLIFETIME(ATDMPlayerState, Deaths);
}

void ATDMPlayerState::OnRep_TeamId() { OnPlayerStatsChanged(); }
void ATDMPlayerState::OnRep_Kills() { OnPlayerStatsChanged(); }
void ATDMPlayerState::OnRep_Deaths() { OnPlayerStatsChanged(); }