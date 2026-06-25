#include "PredictedDashMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


void FSavedMove_Dash::Clear()
{
    Super::Clear();
    bWantsToDash = 0;
}

void FSavedMove_Dash::SetMoveFor(ACharacter* Character, float InDeltaTime, const FVector& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

    UPredictedDashMovementComponent* DashMoveComp = Cast<UPredictedDashMovementComponent>(Character->GetCharacterMovement());
    if (DashMoveComp)
    {
        bWantsToDash = DashMoveComp->bWantsToDash;
    }
}

bool FSavedMove_Dash::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    if (bWantsToDash != ((FSavedMove_Dash*)NewMove.Get())->bWantsToDash)
    {
        return false;
    }
    return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Dash::PrepMoveFor(ACharacter* Character)
{
    Super::PrepMoveFor(Character);

    UPredictedDashMovementComponent* DashMoveComp = Cast<UPredictedDashMovementComponent>(Character->GetCharacterMovement());
    if (DashMoveComp)
    {
        DashMoveComp->bWantsToDash = bWantsToDash;
    }
}

uint8 FSavedMove_Dash::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();
    if (bWantsToDash)
    {
        Result |= FSavedMove_Character::CompressedFlags::FLAG_Reserved_1;
    }
    return Result;
}



FNetworkPredictionData_Client_Dash::FNetworkPredictionData_Client_Dash(const UCharacterMovementComponent& ClientMovement)
    : Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Dash::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_Dash());
}



UPredictedDashMovementComponent::UPredictedDashMovementComponent()
{
    bWantsToDash = false;
    DashImpulse = 2000.0f;
}

FNetworkPredictionData_Client* UPredictedDashMovementComponent::GetPredictionData_Client() const
{
    if (!ClientPredictionData)
    {
        UPredictedDashMovementComponent* MutableThis = const_cast<UPredictedDashMovementComponent*>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Dash(*this);
    }
    return ClientPredictionData;
}

void UPredictedDashMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    bWantsToDash = ((Flags & FSavedMove_Character::CompressedFlags::FLAG_Reserved_1) != 0);
}

void UPredictedDashMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

    if (bWantsToDash && CharacterOwner)
    {
        FVector DashDirection = CharacterOwner->GetActorForwardVector();
        DashDirection.Z = 0.0f;
        DashDirection.Normalize();

        Velocity += DashDirection * DashImpulse;
        bWantsToDash = false; 
    }
}