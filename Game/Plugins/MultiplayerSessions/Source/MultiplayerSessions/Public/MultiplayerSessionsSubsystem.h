// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnStartSession, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnCreateSession, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPOnDestroySession, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMPOnFindSession, TArray<FOnlineSessionSearchResult> SearchResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FMPOnJoinSession, EOnJoinSessionCompleteResult::Type SessionResult, FString Address, bool bWasSuccessful);

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FMPOnStartSession MPOnStartSession;
	FMPOnCreateSession MPOnCreateSession;
	FMPOnDestroySession MPOnDestroySession;
	FMPOnFindSession MPOnFindSession;
	FMPOnJoinSession MPOnJoinSession;

	UMultiplayerSessionsSubsystem();

	void StartGameSession();
	void CreateGameSession(FString MatchType, int32 NumOfConnections);
	void JoinGameSession(const FOnlineSessionSearchResult SearchResult);
	void FindGameSession(int32 MaxSearchResults);
	void DestroyGameSession();

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type SessionResult);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr OnlineSessionInterface;

	FString PrevMatchType; 
	int32 PrevNumOfConnections;

	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle StartSessionDelegateHandle;
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle; 

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
};
