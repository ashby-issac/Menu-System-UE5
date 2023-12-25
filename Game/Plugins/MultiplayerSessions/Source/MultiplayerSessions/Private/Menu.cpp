
#include "Menu.h"
#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.h"

// called from BeginPlay inside Blueprint
void UMenu::MenuSetup(FString TypeOfMatch, int32 NumberOfConnections, FString LobbyPath) // for configuring menu settings
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputMode);
        }
    }

    UGameInstance *GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        MatchType = TypeOfMatch;
        NumOfConnections = NumberOfConnections;
        PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MPOnCreateSession.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MPOnFindSession.AddUObject(this, &ThisClass::OnFindSession);
        MultiplayerSessionsSubsystem->MPOnJoinSession.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MPOnStartSession.AddDynamic(this, &ThisClass::OnStartSession);
        MultiplayerSessionsSubsystem->MPOnDestroySession.AddDynamic(this, &ThisClass::OnDestroySession);
    }
}

bool UMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
    }

    return true;
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if (bWasSuccessful && GEngine)
    {
        GetWorld()->ServerTravel(PathToLobby);
        GEngine->AddOnScreenDebugMessage(-1,
                                         10.f,
                                         FColor::Blue,
                                         FString::Printf(TEXT("Session Created Successfully")));
    }
    else
    {
        HostButton->SetIsEnabled(true);
        GEngine->AddOnScreenDebugMessage(-1,
                                         10.f,
                                         FColor::Red,
                                         FString::Printf(TEXT("Session Creation Failed %s")));
    }
}

void UMenu::OnFindSession(TArray<FOnlineSessionSearchResult> SearchResults, bool bWasSuccessful)
{
    for (auto SearchResult : SearchResults)
    {
        FString MatchTypeTemp;
        SearchResult.Session.SessionSettings.Get("MatchType", MatchTypeTemp);
        if (MatchTypeTemp.Equals(MatchType))
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString(TEXT("3. Found FreeForAll MatchType")));
            MultiplayerSessionsSubsystem->JoinGameSession(SearchResult);
            break;
        }
    }

    JoinButton->SetIsEnabled(!bWasSuccessful || SearchResults.Num() < 1);
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type SessionResult = EOnJoinSessionCompleteResult::SessionDoesNotExist, FString Address = "", bool bWasSuccessful = false)
{
    APlayerController *PlayerController = GetGameInstance()->GetFirstLocalPlayerController(GetWorld());
    if (SessionResult == EOnJoinSessionCompleteResult::Success)
    {
        if (PlayerController)
        {
            PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1,
                                                 10.f,
                                                 FColor::Blue,
                                                 FString(TEXT("Joined Session")));
            }
        }
    }
    else
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    if (HostButton)
    {
        HostButton->SetIsEnabled(true);
    }

    if (JoinButton)
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnLevelRemovedFromWorld(ULevel *InLevel, UWorld *InWorld)
{
    RemoveMenuWidgets();
}

void UMenu::RemoveMenuWidgets()
{
    RemoveFromParent();

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly GameInputMode;
            PlayerController->SetInputMode(GameInputMode);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}

void UMenu::OnHostButtonClicked()
{
    if (!HostButton)
    {
        return;
    }

    HostButton->SetIsEnabled(false);
    MultiplayerSessionsSubsystem->CreateGameSession(MatchType, NumOfConnections);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1,
                                         10.f,
                                         FColor::Blue,
                                         FString(TEXT("Host Button Clicked")));
    }
}

void UMenu::OnJoinButtonClicked()
{
    if (!JoinButton)
    {
        return;
    }

    JoinButton->SetIsEnabled(false);
    MultiplayerSessionsSubsystem->FindGameSession(MaxSearchResults);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1,
                                         10.f,
                                         FColor::Blue,
                                         FString(TEXT("Join Button Clicked")));
    }
}