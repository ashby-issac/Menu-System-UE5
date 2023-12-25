// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(FString TypeOfMatch = "FreeForAll", int32 NumberOfConnections = 4, FString LobbyPath = "/Game/ThirdPerson/Maps/Lobby");

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	void OnFindSession(TArray<class FOnlineSessionSearchResult> SearchResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type SessionResult, FString Address, bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;	

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	FString MatchType, PathToLobby;
    int32 NumOfConnections;
	int32 MaxSearchResults = 10000;

	void RemoveMenuWidgets();

	UFUNCTION()
	void OnHostButtonClicked();	
	
	UFUNCTION()
	void OnJoinButtonClicked();	
};
