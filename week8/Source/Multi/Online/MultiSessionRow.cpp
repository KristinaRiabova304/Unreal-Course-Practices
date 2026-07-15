// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiSessionRow.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

TSharedRef<SWidget> UMultiSessionRow::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UHorizontalBox* Root = WidgetTree->ConstructWidget<UHorizontalBox>();
		WidgetTree->RootWidget = Root;

		InfoText = WidgetTree->ConstructWidget<UTextBlock>();
		InfoText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		if (UHorizontalBoxSlot* S = Root->AddChildToHorizontalBox(InfoText))
		{
			S->SetVerticalAlignment(VAlign_Center);
			S->SetPadding(FMargin(10.f, 4.f));
		}

		JoinButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* JoinLabel = WidgetTree->ConstructWidget<UTextBlock>();
		JoinLabel->SetText(FText::FromString(TEXT("Join")));
		JoinLabel->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		JoinButton->AddChild(JoinLabel);
		JoinButton->OnClicked.AddDynamic(this, &UMultiSessionRow::OnJoinClicked);
		if (UHorizontalBoxSlot* S = Root->AddChildToHorizontalBox(JoinButton))
		{
			S->SetVerticalAlignment(VAlign_Center);
			S->SetPadding(FMargin(10.f, 4.f));
		}

		ApplyLabel();
	}

	return Super::RebuildWidget();
}

void UMultiSessionRow::Setup(UMultiSessionSubsystem* InSessions, const FMultiSessionInfo& InInfo)
{
	Sessions = InSessions;
	SearchIndex = InInfo.SearchIndex;
	CachedLabel = FString::Printf(TEXT("%s    %d/%d    %d ms"),
		InInfo.OwnerName.IsEmpty() ? TEXT("Session") : *InInfo.OwnerName,
		InInfo.CurrentPlayers, InInfo.MaxPlayers, InInfo.PingMs);
	ApplyLabel();
}

void UMultiSessionRow::ApplyLabel()
{
	if (InfoText)
	{
		InfoText->SetText(FText::FromString(CachedLabel));
	}
}

void UMultiSessionRow::OnJoinClicked()
{
	if (Sessions && SearchIndex >= 0)
	{
		if (JoinButton)
		{
			JoinButton->SetIsEnabled(false);
		}
		Sessions->JoinSessionByIndex(SearchIndex);
	}
}
