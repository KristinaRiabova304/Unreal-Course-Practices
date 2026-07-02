#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TDMGamePlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class TDMGAME_API ATDMGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlayingState() override;
	void OnDebugKillPressed();

	
	UFUNCTION(Server, Reliable)
	void Server_DebugKill();

protected:
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardWidgetClass;

	
	UPROPERTY()
	UUserWidget* ScoreboardWidgetInstance;

	
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	bool ShouldUseTouchControls() const;
};