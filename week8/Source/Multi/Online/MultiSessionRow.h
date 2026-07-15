// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiSessionSubsystem.h"
#include "MultiSessionRow.generated.h"

class UButton;
class UTextBlock;

/**
 *  One row in the server browser. Fully built in C++ (no Widget Blueprint needed).
 *  Shows "owner  cur/max  ping ms" and a Join button that joins this row's session.
 */
UCLASS()
class MULTI_API UMultiSessionRow : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fill the row with one search result and wire the Join button to it. */
	void Setup(UMultiSessionSubsystem* InSessions, const FMultiSessionInfo& InInfo);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	UButton* JoinButton = nullptr;

	UPROPERTY()
	UTextBlock* InfoText = nullptr;

	UFUNCTION()
	void OnJoinClicked();

	void ApplyLabel();

	UPROPERTY()
	UMultiSessionSubsystem* Sessions = nullptr;

	int32 SearchIndex = -1;
	FString CachedLabel = TEXT("Session");
};
