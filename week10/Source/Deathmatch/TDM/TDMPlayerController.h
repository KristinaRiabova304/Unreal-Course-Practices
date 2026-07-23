// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeathmatchPlayerController.h"
#include "AntiCheatComponent.h"
#include "TDMPlayerController.generated.h"

class UTDMScoreboardWidget;
class UAntiCheatComponent;

/**
 *  Deathmatch player controller.
 *   - Creates the UMG scoreboard on the owning client from BeginPlayingState().
 *   - Provides a simple "fire" (left mouse) that runs a server-side hitscan so players
 *     can actually kill each other and exercise the scoring path.
 *
 *  Week 10 hardening: the fire and reload RPCs are rate limited and every request is validated
 *  on the server before it can change game state (see ServerFire_Implementation).
 */
UCLASS()
class ATDMPlayerController : public ADeathmatchPlayerController
{
	GENERATED_BODY()

public:

	ATDMPlayerController();

	/** Optional Blueprint override for the scoreboard; falls back to the C++ widget. */
	UPROPERTY(EditDefaultsOnly, Category = "TDM")
	TSubclassOf<UTDMScoreboardWidget> ScoreboardWidgetClass;

protected:

	/** Entering the "Playing" state = the right moment to build & show the scoreboard. */
	virtual void BeginPlayingState() override;

	virtual void SetupInputComponent() override;

	/** Local input handlers. */
	void OnFirePressed();
	void OnReloadPressed();

	/**
	 *  Server-side hitscan that applies lethal damage to the character under the crosshair.
	 *  Takes the client's *claimed* aim so the server can detect and reject tampering; the
	 *  authoritative trace is always performed from the server's own view point.
	 */
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector ClientAimOrigin, FVector ClientAimDir);

	/** Server-side reload request (refills the magazine). Rate limited + state validated. */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	/** Per-player, per-action rate limiter. Owned here so limits are per player. */
	UPROPERTY(VisibleAnywhere, Category = "TDM|AntiCheat")
	TObjectPtr<UAntiCheatComponent> AntiCheat;

	//~ Rate-limit configuration (per action). Defaults set in the constructor.
	UPROPERTY(EditDefaultsOnly, Category = "TDM|AntiCheat")
	FRpcRateLimit FireRateLimit;

	UPROPERTY(EditDefaultsOnly, Category = "TDM|AntiCheat")
	FRpcRateLimit ReloadRateLimit;

	//~ Input-validation tolerances.

	/** Max allowed angle (deg) between the client's claimed aim and the server view direction. */
	UPROPERTY(EditDefaultsOnly, Category = "TDM|AntiCheat", meta = (ClampMin = "0.0"))
	float MaxAimDeviationDegrees = 25.0f;

	/** Max allowed distance (cm) between the client's claimed muzzle origin and the server view. */
	UPROPERTY(EditDefaultsOnly, Category = "TDM|AntiCheat", meta = (ClampMin = "0.0"))
	float MaxAimOriginError = 250.0f;

	/** Range of the fire hitscan (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "TDM")
	float FireRange = 10000.0f;

	/** Damage dealt per shot (>= MaxHealth means a one-shot kill). */
	UPROPERTY(EditDefaultsOnly, Category = "TDM")
	float FireDamage = 100.0f;

private:

	UPROPERTY()
	TObjectPtr<UTDMScoreboardWidget> ScoreboardWidget;
};
