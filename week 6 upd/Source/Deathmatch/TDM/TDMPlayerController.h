// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeathmatchPlayerController.h"
#include "TDMPlayerController.generated.h"

class UTDMScoreboardWidget;

/**
 *  Deathmatch player controller.
 *   - Creates the UMG scoreboard on the owning client from BeginPlayingState().
 *   - Provides a simple "fire" (left mouse) that runs a server-side hitscan so players
 *     can actually kill each other and exercise the scoring path.
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

	/** Local input handler: fire. */
	void OnFirePressed();

	/** Server-side hitscan that applies lethal damage to the character under the crosshair. */
	UFUNCTION(Server, Reliable)
	void ServerFire();

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
