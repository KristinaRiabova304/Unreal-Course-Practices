#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LagRewindWeapon.generated.h"

UCLASS()
class LAGPROJECT_API ALagRewindWeapon : public AActor
{
    GENERATED_BODY()

public:
    ALagRewindWeapon();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFireWeapon(FVector StartLocation, FVector AimDirection, float ClientTimeStamp);

private:
    void DrawDebugHitbox(FVector Location, double Radius, bool bHit);
};