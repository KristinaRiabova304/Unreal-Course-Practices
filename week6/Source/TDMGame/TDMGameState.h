#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TDMGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeamScoresChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRemainingMatchTimeChangedSignature);

UCLASS()
class TDMGAME_API ATDMGameState : public AGameState
{
    GENERATED_BODY()

public:
    ATDMGameState();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  
    UPROPERTY(ReplicatedUsing = OnRep_TeamScores, BlueprintReadOnly, Category = "TDM")
    TArray<int32> TeamScores;

 
    UPROPERTY(ReplicatedUsing = OnRep_RemainingMatchTime, BlueprintReadOnly, Category = "TDM")
    float RemainingMatchTime;

   
    UPROPERTY(BlueprintAssignable, Category = "TDM")
    FOnTeamScoresChangedSignature OnTeamScoresChanged;

    UPROPERTY(BlueprintAssignable, Category = "TDM")
    FOnRemainingMatchTimeChangedSignature OnRemainingMatchTimeChanged;

protected:
    UFUNCTION() void OnRep_TeamScores();
    UFUNCTION() void OnRep_RemainingMatchTime();
};