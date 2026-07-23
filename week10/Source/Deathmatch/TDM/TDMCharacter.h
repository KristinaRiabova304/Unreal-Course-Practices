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

	explicit ATDMCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Standard engine damage entry point. Only does work on the server. */
	virtual float TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDM")
	float MaxHealth = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TDM")
	float Health = 100.0f;

	/** True once the pawn has been scored dead (used by RPC validation to reject "dead man's" fire). */
	bool IsDead() const { return bIsDead; }

	//~ Server-authoritative ammunition. Ammo lives on the pawn so it resets automatically on
	//~ respawn (the pawn is destroyed and re-created). Mutators are server-only.

	/** Rounds currently in the magazine. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TDM")
	int32 CurrentAmmo = 30;

	/** Magazine capacity; also the value RefillMagazine() restores to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDM")
	int32 MagazineSize = 30;

	bool HasAmmo() const { return CurrentAmmo > 0; }
	bool IsMagazineFull() const { return CurrentAmmo >= MagazineSize; }

	/** Server: consume one round (no-op without authority or when empty). */
	void ConsumeAmmo();

	/** Server: refill to MagazineSize (no-op without authority). */
	void RefillMagazine();

protected:

	virtual void BeginPlay() override;

	/** Server: resolve the kill through the GameMode. */
	void Die(AController* Killer);

	/** Guards against scoring the same pawn's death twice. */
	bool bIsDead = false;
};
