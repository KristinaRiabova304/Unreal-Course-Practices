// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiGameInstance.h"
#include "MultiSessionMenu.h"
#include "Multi.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

void UMultiGameInstance::OnStart()
{
	Super::OnStart();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Only show the menu on the initial standalone launch. After hosting (listen server)
	// or joining (client), we don't want the menu to reappear.
	if (World->GetNetMode() != NM_Standalone)
	{
		return;
	}

	Menu = CreateWidget<UMultiSessionMenu>(this, UMultiSessionMenu::StaticClass());
	if (Menu)
	{
		Menu->MenuSetup(4, LobbyMapPath);
		UE_LOG(LogMulti, Log, TEXT("MultiGameInstance: session menu shown at start"));
	}
}
