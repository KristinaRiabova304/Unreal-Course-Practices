#include "TDMGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "TDMGame.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "TDMGameMode.h"

void ATDMGamePlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	
	if (IsLocalPlayerController() && ScoreboardWidgetClass)
	{
		ScoreboardWidgetInstance = CreateWidget<UUserWidget>(this, ScoreboardWidgetClass);
		if (ScoreboardWidgetInstance)
		{
			ScoreboardWidgetInstance->AddToViewport(); 
		}
	}
}

void ATDMGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogTDMGame, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void ATDMGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (IsLocalPlayerController())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
            {
                Subsystem->AddMappingContext(CurrentContext, 0);
            }

            if (!ShouldUseTouchControls())
            {
                for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
                {
                    Subsystem->AddMappingContext(CurrentContext, 0);
                }
            }
        }


        if (InputComponent)
        {
            InputComponent->BindKey(EKeys::K, IE_Pressed, this, &ATDMGamePlayerController::OnDebugKillPressed);
        }
    }
}


void ATDMGamePlayerController::OnDebugKillPressed()
{
   
    Server_DebugKill();
}


void ATDMGamePlayerController::Server_DebugKill_Implementation()
{
   
    ATDMGameMode* GM = GetWorld()->GetAuthGameMode<ATDMGameMode>();
    if (GM)
    {
        AController* Victim = nullptr;

      
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (PC && PC != this) 
            {
                Victim = PC;
                break; 
            }
        }

        GM->ScoreKill(Victim, this);
    }
}

bool ATDMGamePlayerController::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}