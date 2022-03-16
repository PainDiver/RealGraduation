// Fill out your copyright notice in the Description page of Project Settings.


#include "MyReadyRoomGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "ReadyRoomGameStateBase.h"
#include "Blueprint/UserWidget.h"
#include "../Common/MyCharacter.h"
#include "Net/UnrealNetWork.h"
#include "ReadyRoomPlayerController.h"
#include "../PlayMap/MyPlayerState.h"
#include "../Common/MyGameInstance.h"
#include "ReadyRoomPlayerState.h"
#include "GameFrameWork/GameStateBase.h"


#include "Net/OnlineEngineInterface.h"
#include "GameFramework/OnlineSession.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

const static uint8 MIN_PLAYER = 2;

AMyReadyRoomGameModeBase::AMyReadyRoomGameModeBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//_userNum=0;
	//_maxNum = 8;
	

	PlayerControllerClass = AReadyRoomPlayerController::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	GameStateClass = AReadyRoomGameStateBase::StaticClass();
	DefaultPawnClass = AMyCharacter::StaticClass();
	
}

// Called when the game starts or when spawned
void AMyReadyRoomGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	auto gameStateBase = GetGameState<AReadyRoomGameStateBase>();
	if (gameStateBase)
	{
		if (gameStateBase->PlayerArray.Num() >= MIN_PLAYER)
		{
			if (!gameStateBase->GetIsStartable())
				gameStateBase->SetIsStartable(true);
		}
	}
}

void AMyReadyRoomGameModeBase::Tick(float DeltaTimer)
{
	Super::Tick(DeltaTimer);
	
	//_gameInstance->UpdateGameSession(_gameInstance->_currentSessionName);
}

void AMyReadyRoomGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
	if (!(_gameInstance->CheckOpenPublicConnection(true)))
	{
		return;
	}
	
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}



APlayerController* AMyReadyRoomGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	//_userNum++;
	//if (_userNum <= _maxNum)
	//{
	/*UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	instance->UpdateGameSession(instance->_currentSessionName, true, false);*/

	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	//}
	//_userNum--;

	//return NULL;
}


void AMyReadyRoomGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	auto gameStateBase = GetGameState<AReadyRoomGameStateBase>();

	if (gameStateBase)
	{
		if (gameStateBase->PlayerArray.Num() >= MIN_PLAYER)
		{
			if (!gameStateBase->GetIsStartable())
				gameStateBase->SetIsStartable(true);
		}
	}
}



void AMyReadyRoomGameModeBase::Logout(AController* Exiting)
{

	auto gameStateBase = GetGameState<AReadyRoomGameStateBase>();
	if (gameStateBase)
	{
		if (gameStateBase->PlayerArray.Num() - 1 < MIN_PLAYER)
		{
			if (gameStateBase->GetIsStartable())
				gameStateBase->SetIsStartable(false);
		}
	}


	if (!Exiting->GetNetOwningPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("Leaving Map Start"));
		_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
		_gameInstance->CheckOpenPublicConnection(false);
		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "exiting");
		UE_LOG(LogTemp, Warning, TEXT("Leaving Map Done"));
	}
	

	Super::Logout(Exiting);
}


