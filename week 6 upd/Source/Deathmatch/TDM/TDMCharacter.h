// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeathmatchCharacter.h"
#include "TDMCharacter.generated.h"

/**
 *  Deathmatch pawn. Reuses the third-person template character (camera, movement, input)
 *  and adds a minimal server-authoritative health / death path so kills can be scored.
 *
 *  Damage handling lives here, but the *scoring* decision is delegated to the GameMode
 *  (the only class with authority over match state) via ATDMGameMode::ScoreKill().
 */
UCLASS()
class ATDMCharacter : public ADeathmatchCharacter
{
	GENERATED_BODY()

public:

	ATDMCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Standard engine damage entry point. Only does work on the server. */
	virtual float TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDM")
	float MaxHealth = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TDM")
	float Health = 100.0f;

protected:

	virtual void BeginPlay() override;

	/** Server: resolve the kill through the GameMode. */
	void Die(AController* Killer);

	/** Guards against scoring the same pawn's death twice. */
	bool bIsDead = false;
};
