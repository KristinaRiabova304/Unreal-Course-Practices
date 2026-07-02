#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TDMPlayerState.generated.h"

UCLASS()
class TDMGAME_API ATDMPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ATDMPlayerState();


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing = OnRep_TeamId, BlueprintReadOnly, Category = "TDM")
    int32 TeamId;

  
    UPROPERTY(ReplicatedUsing = OnRep_Kills, BlueprintReadOnly, Category = "TDM")
    int32 Kills;

    UPROPERTY(ReplicatedUsing = OnRep_Deaths, BlueprintReadOnly, Category = "TDM")
    int32 Deaths;

  
    UFUNCTION(BlueprintImplementableEvent, Category = "TDM")
    void OnPlayerStatsChanged();

protected:
  
    UFUNCTION() void OnRep_TeamId();
    UFUNCTION() void OnRep_Kills();
    UFUNCTION() void OnRep_Deaths();
};