// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiSessionSubsystem.generated.h"

/** Lightweight, Blueprint-friendly view of a single found session */
USTRUCT(BlueprintType)
struct FMultiSessionInfo
{
	GENERATED_BODY()

	/** Index into the internal search results, used to join */
	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 SearchIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FString OwnerName;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 PingMs = 0;
};

/** Multicast delegates the UI binds to (all Blueprint-assignable) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiOnFindSessionsComplete, const TArray<FMultiSessionInfo>&, Results, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiOnJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiOnDestroySessionComplete, bool, bWasSuccessful);

/**
 *  GameInstance subsystem that wraps the Online Session interface (Steam).
 *  Handles create / find / join / destroy and reports back through Blueprint delegates.
 */
UCLASS()
class MULTI_API UMultiSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiSessionSubsystem();

	//~ Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem

	/** Host a new session. NumPublicConnections = max players. bIsLANMatch should be false for internet play. */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(int32 NumPublicConnections, bool bIsLANMatch = false);

	/** Search for open sessions. Results are returned through OnFindSessionsComplete. */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions(int32 MaxSearchResults = 50, bool bIsLANQuery = false);

	/** Join a session previously returned by FindSessions, by its SearchIndex. */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSessionByIndex(int32 SearchIndex);

	/** Tear the current session down (call before hosting a new one). */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	/** Level (with ?listen appended) the host travels to once the session is created. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString LobbyMapPath = TEXT("/Game/ThirdPerson/Lvl_ThirdPerson");

	/** Keyword used to advertise/filter our sessions so we don't join unrelated AppId 480 lobbies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString MatchKeyword = TEXT("MultiFreeForAll");

	// Delegates the UI listens on
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMultiOnCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMultiOnFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMultiOnJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FMultiOnDestroySessionComplete OnDestroySessionComplete;

private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSearch;

	// Native OSS delegates + their handles
	FOnCreateSessionCompleteDelegate CreateCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroyCompleteDelegate;

	FDelegateHandle CreateCompleteDelegateHandle;
	FDelegateHandle FindCompleteDelegateHandle;
	FDelegateHandle JoinCompleteDelegateHandle;
	FDelegateHandle DestroyCompleteDelegateHandle;

	// If a session already exists, destroy it and re-create with these pending settings
	bool bCreateOnDestroy = false;
	int32 PendingNumConnections = 4;
	bool bPendingIsLAN = false;

	// Native OSS callbacks
	void OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleteInternal(bool bWasSuccessful);
	void OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful);
};
