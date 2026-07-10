#include "LagRewindWeapon.h"
#include "LagprojectCharacter.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

static TAutoConsoleVariable<int32> CVarDrawRewindDebug(
    TEXT("vfx.DrawRewindDebug"),
    0,
    TEXT("1 = Show rewound hitboxes on the server, 0 = Disabled"),
    ECVF_Cheat
);

ALagRewindWeapon::ALagRewindWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

bool ALagRewindWeapon::ServerFireWeapon_Validate(FVector StartLocation, FVector AimDirection, float ClientTimeStamp)
{
    if (AimDirection.IsNearlyZero() || !AimDirection.IsNormalized())
    {
        return false;
    }

    if (!GetWorld() || !GetWorld()->GetGameState()) return false;

    float CurrentServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
    float TimeDelta = CurrentServerTime - ClientTimeStamp;

    if (TimeDelta < 0.0f || TimeDelta > 0.250f)
    {
        return false;
    }

    return true;
}

void ALagRewindWeapon::ServerFireWeapon_Implementation(FVector StartLocation, FVector AimDirection, float ClientTimeStamp)
{
    double TraceDistance = 10000.0;
    FVector EndLocation = StartLocation + (AimDirection * TraceDistance);

    for (TActorIterator<ALagprojectCharacter> It(GetWorld()); It; ++It)
    {
        ALagprojectCharacter* TargetChar = *It;
        if (!TargetChar || TargetChar == GetOwner()) continue;

        FTransform RewoundHead, RewoundTorso;
        if (TargetChar->GetInterpolatedTransforms(ClientTimeStamp, RewoundHead, RewoundTorso))
        {
            double HeadRadius = 18.0;
            double TorsoRadius = 35.0;

            bool bHitHead = FMath::LineSphereIntersection(StartLocation, AimDirection, TraceDistance, RewoundHead.GetLocation(), HeadRadius);
            bool bHitTorso = FMath::LineSphereIntersection(StartLocation, AimDirection, TraceDistance, RewoundTorso.GetLocation(), TorsoRadius);

            if (bHitHead || bHitTorso)
            {
                UGameplayStatics::ApplyDamage(TargetChar, 20.0f, GetInstigatorController(), this, UDamageType::StaticClass());

                if (bHitHead) DrawDebugHitbox(RewoundHead.GetLocation(), HeadRadius, true);
                if (bHitTorso) DrawDebugHitbox(RewoundTorso.GetLocation(), TorsoRadius, true);
                break;
            }
            else
            {
                DrawDebugHitbox(RewoundHead.GetLocation(), HeadRadius, false);
                DrawDebugHitbox(RewoundTorso.GetLocation(), TorsoRadius, false);
            }
        }
    }
}

void ALagRewindWeapon::DrawDebugHitbox(FVector Location, double Radius, bool bHit)
{
    if (CVarDrawRewindDebug.GetValueOnGameThread() > 0)
    {
        FColor Color = bHit ? FColor::Green : FColor::Red;
        DrawDebugSphere(GetWorld(), Location, static_cast<float>(Radius), 12, Color, false, 2.0f);
    }
}