// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiSessionSubsystem.h"
#include "MultiSessionMenu.generated.h"

class UButton;
class USpinBox;
class UTextBlock;
class UScrollBox;
class UMultiSessionRow;

/**
 *  Host + server-browser menu, fully built in C++ (no Widget Blueprint required).
 *  Call MenuSetup() to show it (done automatically by UMultiGameInstance::OnStart).
 */
UCLASS()
class MULTI_API UMultiSessionMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMultiSessionMenu(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Session|Menu")
	void MenuSetup(int32 InDefaultMaxPlayers = 4, FString InLobbyPath = TEXT("/Game/ThirdPerson/Lvl_ThirdPerson"));

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	/** Row widget class used to populate the browser. Defaults to UMultiSessionRow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session|Menu")
	TSubclassOf<UMultiSessionRow> RowWidgetClass;

private:
	UPROPERTY()
	UButton* HostButton = nullptr;

	UPROPERTY()
	UButton* RefreshButton = nullptr;

	UPROPERTY()
	UScrollBox* SessionList = nullptr;

	UPROPERTY()
	USpinBox* MaxPlayersSpinBox = nullptr;

	UPROPERTY()
	UTextBlock* StatusText = nullptr;

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void HandleCreateComplete(bool bWasSuccessful);

	UFUNCTION()
	void HandleFindComplete(const TArray<FMultiSessionInfo>& Results, bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinComplete(bool bWasSuccessful);

	void BindToSubsystem();
	void SetButtonsEnabled(bool bEnabled);
	void SetStatus(const FString& Message);
	void RemoveMenu();

	UPROPERTY()
	UMultiSessionSubsystem* Sessions = nullptr;

	int32 DefaultMaxPlayers = 4;
	FString LobbyPath = TEXT("/Game/ThirdPerson/Lvl_ThirdPerson");
};
