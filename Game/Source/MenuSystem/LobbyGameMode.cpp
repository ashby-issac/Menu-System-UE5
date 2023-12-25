// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController *PlayerController)
{
    Super::PostLogin(PlayerController);

    if (GameState)
    {
        int32 NumPlayers = GameState.Get()->PlayerArray.Num();
        APlayerState *PlayerState = PlayerController->GetPlayerState<APlayerState>();
        FString PlayerName = PlayerState->GetPlayerName();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             20.f,
                                             FColor::Blue,
                                             FString::Printf(TEXT("Number of players joined: %d"), (NumPlayers)));
            GEngine->AddOnScreenDebugMessage(1,
                                             20.f,
                                             FColor::Green,
                                             FString::Printf(TEXT("%s has joined"), *PlayerName));
        }
    }
}

void ALobbyGameMode::Logout(AController *Controller)
{
    Super::Logout(Controller);

    if (GameState)
    {
        int32 NumPlayers = GameState.Get()->PlayerArray.Num();
        APlayerState *PlayerState = Controller->GetPlayerState<APlayerState>();
        FString PlayerName = PlayerState->GetPlayerName();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             20.f,
                                             FColor::Blue,
                                             FString::Printf(TEXT("Number of players left: %d"), (NumPlayers - 1)));
            GEngine->AddOnScreenDebugMessage(1,
                                             20.f,
                                             FColor::Green,
                                             FString::Printf(TEXT("%s has left"), *PlayerName));
        }
    }
}
