// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDMScoreboardWidget.h"
#include "TDMGameState.h"
#include "TDMPlayerState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"

TSharedRef<SWidget> UTDMScoreboardWidget::RebuildWidget()
{
	// Build the widget tree in code so no .uasset is required.
	RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
	WidgetTree->RootWidget = RootBox;

	HeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HeaderText"));
	BodyText   = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BodyText"));

	if (RootBox)
	{
		RootBox->AddChildToVerticalBox(HeaderText);
		RootBox->AddChildToVerticalBox(BodyText);
	}

	return Super::RebuildWidget();
}

void UTDMScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Anchor to the top-left corner of the screen.
	SetPositionInViewport(FVector2D(40.0f, 40.0f), false);

	if (HeaderText)
	{
		HeaderText->SetText(FText::FromString(TEXT("TEAM DEATHMATCH")));
	}

	TryBindGameState();
}

void UTDMScoreboardWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryHandle);
	}

	if (ATDMGameState* GS = BoundGameState.Get())
	{
		GS->OnScoresChanged.RemoveAll(this);
		GS->OnTimeChanged.RemoveAll(this);
	}

	for (const TWeakObjectPtr<ATDMPlayerState>& WeakPS : BoundPlayerStates)
	{
		if (ATDMPlayerState* PS = WeakPS.Get())
		{
			PS->OnPlayerStatsChanged.RemoveAll(this);
		}
	}
	BoundPlayerStates.Reset();

	Super::NativeDestruct();
}

void UTDMScoreboardWidget::TryBindGameState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ATDMGameState* GS = World->GetGameState<ATDMGameState>();
	if (!GS)
	{
		// GameState hasn't replicated yet — retry shortly. This is one-time hookup,
		// not a data-refresh poll.
		World->GetTimerManager().SetTimer(BindRetryHandle, this, &UTDMScoreboardWidget::TryBindGameState, 0.25f, false);
		return;
	}

	BoundGameState = GS;
	GS->OnScoresChanged.AddUObject(this, &UTDMScoreboardWidget::Refresh);
	GS->OnTimeChanged.AddUObject(this, &UTDMScoreboardWidget::Refresh);

	EnsurePlayerStateBindings();
	Refresh();
}

void UTDMScoreboardWidget::EnsurePlayerStateBindings()
{
	ATDMGameState* GS = BoundGameState.Get();
	if (!GS)
	{
		return;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		ATDMPlayerState* TPS = Cast<ATDMPlayerState>(PS);
		if (!TPS)
		{
			continue;
		}

		const bool bAlreadyBound = BoundPlayerStates.ContainsByPredicate(
			[TPS](const TWeakObjectPtr<ATDMPlayerState>& W) { return W.Get() == TPS; });

		if (!bAlreadyBound)
		{
			TPS->OnPlayerStatsChanged.AddUObject(this, &UTDMScoreboardWidget::Refresh);
			BoundPlayerStates.Add(TPS);
		}
	}
}

void UTDMScoreboardWidget::Refresh()
{
	ATDMGameState* GS = BoundGameState.Get();
	if (!GS)
	{
		return;
	}

	// A time/score tick is also our cue to pick up any newly-joined players.
	EnsurePlayerStateBindings();

	// Header: timer + team totals.
	FString Header = FString::Printf(TEXT("TEAM DEATHMATCH   %s\nRED  %d      BLUE  %d"),
		*FormatTime(GS->RemainingMatchTime), GS->GetTeamScore(0), GS->GetTeamScore(1));

	if (GS->HasMatchEnded())
	{
		const int32 Winner = GS->GetWinningTeam();
		const TCHAR* Result = (Winner < 0) ? TEXT("DRAW") : (Winner == 0 ? TEXT("RED WINS") : TEXT("BLUE WINS"));
		Header += FString::Printf(TEXT("\n--- MATCH OVER: %s ---"), Result);
	}

	if (HeaderText)
	{
		HeaderText->SetText(FText::FromString(Header));
	}

	// Body: one row per player, from the replicated PlayerArray.
	FString Body;
	Body += TEXT("\nPLAYER              TEAM   K   D\n");
	for (APlayerState* PS : GS->PlayerArray)
	{
		const ATDMPlayerState* TPS = Cast<ATDMPlayerState>(PS);
		if (!TPS)
		{
			continue;
		}

		const TCHAR* TeamName = (TPS->TeamId == 0) ? TEXT("RED ") : (TPS->TeamId == 1 ? TEXT("BLUE") : TEXT("----"));
		Body += FString::Printf(TEXT("%-18s  %s  %3d %3d\n"),
			*TPS->GetPlayerName(), TeamName, TPS->Kills, TPS->Deaths);
	}

	if (BodyText)
	{
		BodyText->SetText(FText::FromString(Body));
	}
}

FString UTDMScoreboardWidget::FormatTime(float Seconds)
{
	const int32 Total = FMath::Max(0, FMath::CeilToInt(Seconds));
	return FString::Printf(TEXT("%02d:%02d"), Total / 60, Total % 60);
}
