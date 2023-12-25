// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() : StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
                                                                 CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
                                                                 FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
                                                                 JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
                                                                 DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))

{
    IOnlineSubsystem *OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             10.f,
                                             FColor::Blue,
                                             FString::Printf(TEXT("Online Subsystem Service Provider: %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
        }
    }
}

void UMultiplayerSessionsSubsystem::StartGameSession()
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    StartSessionDelegateHandle = OnlineSessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
    OnlineSessionInterface->StartSession(NAME_GameSession);
}

void UMultiplayerSessionsSubsystem::CreateGameSession(FString MatchType, int32 NumOfConnections)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    FNamedOnlineSession *ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession)
    {
        PrevMatchType = MatchType;
        PrevNumOfConnections = NumOfConnections;
        DestroyGameSession();
        return;
    }

    CreateSessionDelegateHandle = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    SessionSettings = MakeShareable(new FOnlineSessionSettings());
    FName MatchTypeKey = "MatchType";

    SessionSettings->bUsesPresence = true;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bAllowJoinViaPresence = true;
    SessionSettings->bUseLobbiesIfAvailable = true;
    SessionSettings->NumPublicConnections = NumOfConnections;
    SessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
    SessionSettings->BuildUniqueId = 1;

    SessionSettings->Set(MatchTypeKey, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void UMultiplayerSessionsSubsystem::FindGameSession(int32 MaxSearchResults)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    FindSessionsDelegateHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = MaxSearchResults;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void UMultiplayerSessionsSubsystem::JoinGameSession(const FOnlineSessionSearchResult SearchResult)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    JoinSessionDelegateHandle = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
    ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult);
}

void UMultiplayerSessionsSubsystem::DestroyGameSession()
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    DestroySessionDelegateHandle = OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
    OnlineSessionInterface->DestroySession(NAME_GameSession);
}

// Delegate Completion Callback Functions

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!GEngine)
    {
        return;
    }

    OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionDelegateHandle);
    MPOnStartSession.Broadcast(bWasSuccessful);

    if (bWasSuccessful)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Starting session successful")));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString(TEXT("Starting session failed")));
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
    MPOnDestroySession.Broadcast(bWasSuccessful);

    if (bWasSuccessful)
    {
        CreateGameSession(PrevMatchType, PrevNumOfConnections);
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Destroying session successful")));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString(TEXT("Destroying session failed")));
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
    MPOnCreateSession.Broadcast(bWasSuccessful);

    if (bWasSuccessful)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Creating session successful")));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Creating session failed")));
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);

    TArray<FOnlineSessionSearchResult> SearchResults = bWasSuccessful ? SessionSearch->SearchResults : TArray<FOnlineSessionSearchResult>();
    MPOnFindSession.Broadcast(SearchResults, bWasSuccessful);

    if (bWasSuccessful)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("1. Session Found")));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString(TEXT("1. Session Not Found")));
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type SessionResult)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("OnJoinSessionComplete function")));

    FString Address;
    bool isSuccess = OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
    OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);

    if (isSuccess)
    {
        MPOnJoinSession.Broadcast(EOnJoinSessionCompleteResult::Success, Address, isSuccess);
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Joining session successful")));
    }
    else
    {
        MPOnJoinSession.Broadcast(EOnJoinSessionCompleteResult::SessionDoesNotExist, "", isSuccess);
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("Joining session failed")));
    }
}