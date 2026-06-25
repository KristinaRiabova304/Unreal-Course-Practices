#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PredictedDashMovementComponent.generated.h"


class FSavedMove_Dash : public FSavedMove_Character
{
public:
    typedef FSavedMove_Character Super;

    uint8 bWantsToDash : 1;

    virtual void Clear() override;
  
    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, const FVector& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
    virtual void PrepMoveFor(ACharacter* Character) override;
    virtual uint8 GetCompressedFlags() const override;
};


class FNetworkPredictionData_Client_Dash : public FNetworkPredictionData_Client_Character
{
public:
    typedef FNetworkPredictionData_Client_Character Super;

    FNetworkPredictionData_Client_Dash(const UCharacterMovementComponent& ClientMovement);
    virtual FSavedMovePtr AllocateNewMove() override;
};


UCLASS()
class PRACTICE5_API UPredictedDashMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UPredictedDashMovementComponent();

    uint8 bWantsToDash : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
    float DashImpulse;

protected:
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
    virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
};