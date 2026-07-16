// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDMScoreboardWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class ATDMGameState;
class ATDMPlayerState;

/**
 *  Scoreboard driven entirely by replicated state.
 *
 *  It never polls on Tick: it binds to the RepNotify-backed delegates on the GameState
 *  (scores + timer) and on each PlayerState (kills/deaths/team), and rebuilds its text
 *  only when one of those fires. Late joiners are handled because the per-second timer
 *  replication also (re)binds any PlayerState that has appeared since the last update.
 *
 *  Built in pure C++ (no .uasset needed) so it works out of the box.
 */
UCLASS()
class UTDMScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	/** Attempts to bind to the GameState; retries via a short timer until it exists. */
	void TryBindGameState();

	/** Binds any not-yet-bound PlayerState in the current PlayerArray (handles late joiners). */
	void EnsurePlayerStateBindings();

	/** Rebuilds the displayed text from current replicated values. */
	void Refresh();

	static FString FormatTime(float Seconds);

	UPROPERTY()
	TObjectPtr<UVerticalBox> RootBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY()
	TObjectPtr<UTextBlock> BodyText;

	TWeakObjectPtr<ATDMGameState> BoundGameState;

	/** PlayerStates we've already subscribed to. */
	TArray<TWeakObjectPtr<ATDMPlayerState>> BoundPlayerStates;

	FTimerHandle BindRetryHandle;
};
