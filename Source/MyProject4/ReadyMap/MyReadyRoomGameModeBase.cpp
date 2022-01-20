// Fill out your copyright notice in the Description page of Project Settings.


#include "MyReadyRoomGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "ReadyRoomGameStateBase.h"
#include "Blueprint/UserWidget.h"
#include "../Common/MyCharacter.h"
#include "Net/UnrealNetWork.h"
#include "ReadyRoomPlayerController.h"
#include "../PlayMap/MyPlayerState.h"

AMyReadyRoomGameModeBase::AMyReadyRoomGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
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
	
	AGameStateBase* gameState = UGameplayStatics::GetGameState(GetWorld());
	_gameState = Cast<AReadyRoomGameStateBase>(gameState);

}



void AMyReadyRoomGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	//if (_userNum <= _maxNum)
	//{
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	//}
}



APlayerController* AMyReadyRoomGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	//_userNum++;
	//if (_userNum <= _maxNum)
	//{
		return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	//}
	//_userNum--;

	//return NULL;
}

void AMyReadyRoomGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	//_userNum--;
	
}

void AMyReadyRoomGameModeBase::InitSeamlessTravelPlayer(AController* newController)
{
	Super::InitSeamlessTravelPlayer(newController);
}