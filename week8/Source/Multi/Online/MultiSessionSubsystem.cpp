// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiSessionSubsystem.h"
#include "Multi.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

UMultiSessionSubsystem::UMultiSessionSubsystem()
	: CreateCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiSessionSubsystem::OnCreateSessionCompleteInternal))
	, FindCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiSessionSubsystem::OnFindSessionsCompleteInternal))
	, JoinCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiSessionSubsystem::OnJoinSessionCompleteInternal))
	, DestroyCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiSessionSubsystem::OnDestroySessionCompleteInternal))
{
}

void UMultiSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		SessionInterface = OSS->GetSessionInterface();
		UE_LOG(LogMulti, Log, TEXT("MultiSessionSubsystem online service: %s"), *OSS->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogMulti, Warning, TEXT("MultiSessionSubsystem: no online subsystem available"));
	}
}

void UMultiSessionSubsystem::Deinitialize()
{
	SessionInterface.Reset();
	Super::Deinitialize();
}

void UMultiSessionSubsystem::CreateSession(int32 NumPublicConnections, bool bIsLANMatch)
{
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	// A session already exists — destroy it first, then re-create in the destroy callback.
	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		bCreateOnDestroy = true;
		PendingNumConnections = NumPublicConnections;
		bPendingIsLAN = bIsLANMatch;
		DestroySession();
		return;
	}

	CreateCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->NumPublicConnections = FMath::Max(1, NumPublicConnections);
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bIsLANMatch = bIsLANMatch;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->Set(SEARCH_KEYWORDS, MatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (!LocalPlayer || !SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteDelegateHandle);
		UE_LOG(LogMulti, Warning, TEXT("CreateSession call failed to start"));
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiSessionSubsystem::OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteDelegateHandle);
	}

	UE_LOG(LogMulti, Log, TEXT("CreateSession complete: %s"), bWasSuccessful ? TEXT("OK") : TEXT("FAILED"));
	OnCreateSessionComplete.Broadcast(bWasSuccessful);

	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(LobbyMapPath + TEXT("?listen"));
		}
	}
}

void UMultiSessionSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLANQuery)
{
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsComplete.Broadcast(TArray<FMultiSessionInfo>(), false);
		return;
	}

	FindCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindCompleteDelegate);

	LastSearch = MakeShareable(new FOnlineSessionSearch());
	LastSearch->MaxSearchResults = MaxSearchResults;
	LastSearch->bIsLanQuery = bIsLANQuery;
	// Steam sessions are hosted as lobbies (bUseLobbiesIfAvailable), so search lobbies.
	LastSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	// Only surface sessions advertised with our keyword
	LastSearch->QuerySettings.Set(SEARCH_KEYWORDS, MatchKeyword, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (!LocalPlayer || !SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindCompleteDelegateHandle);
		UE_LOG(LogMulti, Warning, TEXT("FindSessions call failed to start"));
		OnFindSessionsComplete.Broadcast(TArray<FMultiSessionInfo>(), false);
	}
}

void UMultiSessionSubsystem::OnFindSessionsCompleteInternal(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindCompleteDelegateHandle);
	}

	TArray<FMultiSessionInfo> Infos;
	if (bWasSuccessful && LastSearch.IsValid())
	{
		for (int32 i = 0; i < LastSearch->SearchResults.Num(); ++i)
		{
			const FOnlineSessionSearchResult& Result = LastSearch->SearchResults[i];

			FMultiSessionInfo Info;
			Info.SearchIndex = i;
			Info.OwnerName = Result.Session.OwningUserName;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
			Info.PingMs = Result.PingInMs;
			Infos.Add(Info);
		}
	}

	UE_LOG(LogMulti, Log, TEXT("FindSessions complete: %d result(s)"), Infos.Num());
	OnFindSessionsComplete.Broadcast(Infos, bWasSuccessful);
}

void UMultiSessionSubsystem::JoinSessionByIndex(int32 SearchIndex)
{
	if (!SessionInterface.IsValid() || !LastSearch.IsValid() || !LastSearch->SearchResults.IsValidIndex(SearchIndex))
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	JoinCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (!LocalPlayer || !SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, LastSearch->SearchResults[SearchIndex]))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinCompleteDelegateHandle);
		UE_LOG(LogMulti, Warning, TEXT("JoinSession call failed to start"));
		OnJoinSessionComplete.Broadcast(false);
	}
}

void UMultiSessionSubsystem::OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinCompleteDelegateHandle);
	}

	const bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
	UE_LOG(LogMulti, Log, TEXT("JoinSession complete: %s"), bSuccess ? TEXT("OK") : TEXT("FAILED"));
	OnJoinSessionComplete.Broadcast(bSuccess);

	if (!bSuccess)
	{
		return;
	}

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString))
	{
		if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
		{
			UE_LOG(LogMulti, Log, TEXT("Client travel to %s"), *ConnectString);
			PC->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
}

void UMultiSessionSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		OnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroyCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroyCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteDelegateHandle);
		OnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiSessionSubsystem::OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteDelegateHandle);
	}

	OnDestroySessionComplete.Broadcast(bWasSuccessful);

	if (bWasSuccessful && bCreateOnDestroy)
	{
		bCreateOnDestroy = false;
		CreateSession(PendingNumConnections, bPendingIsLAN);
	}
}
