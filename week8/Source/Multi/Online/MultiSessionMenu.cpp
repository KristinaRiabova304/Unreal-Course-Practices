// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiSessionMenu.h"
#include "MultiSessionRow.h"
#include "Multi.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

UMultiSessionMenu::UMultiSessionMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default the browser rows to the C++ row widget (no Blueprint needed)
	RowWidgetClass = UMultiSessionRow::StaticClass();
}

TSharedRef<SWidget> UMultiSessionMenu::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		// Dark translucent backdrop, centred on screen
		UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
		Bg->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.6f));
		Bg->SetHorizontalAlignment(HAlign_Center);
		Bg->SetVerticalAlignment(VAlign_Center);
		Bg->SetPadding(FMargin(24.f));
		WidgetTree->RootWidget = Bg;

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		Bg->SetContent(Column);

		// Title
		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("Multi — Sessions")));
		Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Title))
		{
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetPadding(FMargin(4.f, 8.f));
		}

		// Row: "Max players" label + spin box + Host button
		UHorizontalBox* HostRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(HostRow))
		{
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetPadding(FMargin(4.f, 6.f));
		}

		UTextBlock* MaxLabel = WidgetTree->ConstructWidget<UTextBlock>();
		MaxLabel->SetText(FText::FromString(TEXT("Max players")));
		MaxLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		if (UHorizontalBoxSlot* S = HostRow->AddChildToHorizontalBox(MaxLabel))
		{
			S->SetVerticalAlignment(VAlign_Center);
			S->SetPadding(FMargin(6.f, 0.f));
		}

		MaxPlayersSpinBox = WidgetTree->ConstructWidget<USpinBox>();
		MaxPlayersSpinBox->SetMinValue(2);
		MaxPlayersSpinBox->SetMaxValue(16);
		MaxPlayersSpinBox->SetMinSliderValue(2);
		MaxPlayersSpinBox->SetMaxSliderValue(16);
		MaxPlayersSpinBox->SetValue(DefaultMaxPlayers);
		if (UHorizontalBoxSlot* S = HostRow->AddChildToHorizontalBox(MaxPlayersSpinBox))
		{
			S->SetVerticalAlignment(VAlign_Center);
			S->SetPadding(FMargin(6.f, 0.f));
		}

		HostButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* HostLabel = WidgetTree->ConstructWidget<UTextBlock>();
		HostLabel->SetText(FText::FromString(TEXT("Host")));
		HostLabel->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		HostButton->AddChild(HostLabel);
		HostButton->OnClicked.AddDynamic(this, &UMultiSessionMenu::OnHostClicked);
		if (UHorizontalBoxSlot* S = HostRow->AddChildToHorizontalBox(HostButton))
		{
			S->SetVerticalAlignment(VAlign_Center);
			S->SetPadding(FMargin(6.f, 0.f));
		}

		// Refresh button
		RefreshButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* RefreshLabel = WidgetTree->ConstructWidget<UTextBlock>();
		RefreshLabel->SetText(FText::FromString(TEXT("Refresh (find sessions)")));
		RefreshLabel->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		RefreshButton->AddChild(RefreshLabel);
		RefreshButton->OnClicked.AddDynamic(this, &UMultiSessionMenu::OnRefreshClicked);
		if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(RefreshButton))
		{
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetPadding(FMargin(4.f, 6.f));
		}

		// Status line
		StatusText = WidgetTree->ConstructWidget<UTextBlock>();
		StatusText->SetText(FText::FromString(TEXT("Ready")));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.9f, 1.f)));
		if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(StatusText))
		{
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetPadding(FMargin(4.f, 6.f));
		}

		// Session list (rows get added here)
		SessionList = WidgetTree->ConstructWidget<UScrollBox>();
		if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(SessionList))
		{
			S->SetHorizontalAlignment(HAlign_Fill);
			S->SetPadding(FMargin(4.f, 6.f));
		}
	}

	return Super::RebuildWidget();
}

void UMultiSessionMenu::MenuSetup(int32 InDefaultMaxPlayers, FString InLobbyPath)
{
	DefaultMaxPlayers = InDefaultMaxPlayers;
	LobbyPath = InLobbyPath;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (MaxPlayersSpinBox)
	{
		MaxPlayersSpinBox->SetValue(DefaultMaxPlayers);
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}

	BindToSubsystem();
	SetStatus(TEXT("Ready"));
}

void UMultiSessionMenu::BindToSubsystem()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		Sessions = GI->GetSubsystem<UMultiSessionSubsystem>();
		if (Sessions)
		{
			Sessions->LobbyMapPath = LobbyPath;
			Sessions->OnCreateSessionComplete.AddDynamic(this, &UMultiSessionMenu::HandleCreateComplete);
			Sessions->OnFindSessionsComplete.AddDynamic(this, &UMultiSessionMenu::HandleFindComplete);
			Sessions->OnJoinSessionComplete.AddDynamic(this, &UMultiSessionMenu::HandleJoinComplete);
		}
		else
		{
			SetStatus(TEXT("Session subsystem unavailable (Steam not initialized?)."));
		}
	}
}

void UMultiSessionMenu::OnHostClicked()
{
	if (!Sessions)
	{
		return;
	}

	int32 NumPlayers = DefaultMaxPlayers;
	if (MaxPlayersSpinBox)
	{
		NumPlayers = FMath::Max(2, FMath::RoundToInt(MaxPlayersSpinBox->GetValue()));
	}

	SetButtonsEnabled(false);
	SetStatus(FString::Printf(TEXT("Creating session for %d players..."), NumPlayers));
	Sessions->CreateSession(NumPlayers, /*bIsLANMatch*/ false);
}

void UMultiSessionMenu::OnRefreshClicked()
{
	if (!Sessions)
	{
		return;
	}

	if (SessionList)
	{
		SessionList->ClearChildren();
	}

	SetButtonsEnabled(false);
	SetStatus(TEXT("Searching for sessions..."));
	Sessions->FindSessions(50, /*bIsLANQuery*/ false);
}

void UMultiSessionMenu::HandleCreateComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SetStatus(TEXT("Session created — traveling to lobby..."));
		RemoveMenu();
	}
	else
	{
		SetStatus(TEXT("Failed to create session."));
		SetButtonsEnabled(true);
	}
}

void UMultiSessionMenu::HandleFindComplete(const TArray<FMultiSessionInfo>& Results, bool bWasSuccessful)
{
	SetButtonsEnabled(true);

	if (SessionList)
	{
		SessionList->ClearChildren();
	}

	if (!bWasSuccessful || Results.Num() == 0)
	{
		SetStatus(TEXT("No sessions found."));
		return;
	}

	for (const FMultiSessionInfo& Info : Results)
	{
		UMultiSessionRow* Row = CreateWidget<UMultiSessionRow>(this, RowWidgetClass);
		if (Row)
		{
			Row->Setup(Sessions, Info);
			if (SessionList)
			{
				SessionList->AddChild(Row);
			}
		}
	}

	SetStatus(FString::Printf(TEXT("Found %d session(s)."), Results.Num()));
}

void UMultiSessionMenu::HandleJoinComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SetStatus(TEXT("Joined — traveling to host..."));
		RemoveMenu();
	}
	else
	{
		SetStatus(TEXT("Failed to join session."));
		SetButtonsEnabled(true);
	}
}

void UMultiSessionMenu::SetButtonsEnabled(bool bEnabled)
{
	if (HostButton) { HostButton->SetIsEnabled(bEnabled); }
	if (RefreshButton) { RefreshButton->SetIsEnabled(bEnabled); }
}

void UMultiSessionMenu::SetStatus(const FString& Message)
{
	UE_LOG(LogMulti, Log, TEXT("[Menu] %s"), *Message);
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Message));
	}
}

void UMultiSessionMenu::RemoveMenu()
{
	RemoveFromParent();

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			const FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);
		}
	}
}

void UMultiSessionMenu::NativeDestruct()
{
	if (Sessions)
	{
		Sessions->OnCreateSessionComplete.RemoveDynamic(this, &UMultiSessionMenu::HandleCreateComplete);
		Sessions->OnFindSessionsComplete.RemoveDynamic(this, &UMultiSessionMenu::HandleFindComplete);
		Sessions->OnJoinSessionComplete.RemoveDynamic(this, &UMultiSessionMenu::HandleJoinComplete);
	}

	Super::NativeDestruct();
}
