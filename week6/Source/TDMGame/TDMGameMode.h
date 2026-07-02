#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TDMGameMode.generated.h"

UCLASS()
class TDMGAME_API ATDMGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ATDMGameMode();

 
    virtual void PostLogin(APlayerController* NewPlayer) override;

   
    UFUNCTION(BlueprintCallable, Category = "TDM")
    void ScoreKill(AController* VictimController, AController* KillerController);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "TDM")
    float MatchDuration = 180.0f; 

    FTimerHandle MatchTimerHandle;
    void AdvanceTimer();
};