// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MultiGameInstance.generated.h"

class UMultiSessionMenu;

/**
 *  Game instance that shows the session menu automatically at game start
 *  (only in standalone — not when we are already a listen server or client after travel).
 *  Wired via [/Script/EngineSettings.GameMapsSettings] GameInstanceClass in DefaultEngine.ini.
 */
UCLASS()
class MULTI_API UMultiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void OnStart() override;

	/** Map (with ?listen appended by the subsystem) the host travels to. */
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FString LobbyMapPath = TEXT("/Game/ThirdPerson/Lvl_ThirdPerson");

private:
	UPROPERTY()
	UMultiSessionMenu* Menu = nullptr;
};
