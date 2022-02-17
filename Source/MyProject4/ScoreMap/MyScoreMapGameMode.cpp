// Fill out your copyright notice in the Description page of Project Settings.


#include "MyScoreMapGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "MyScoreMapGameStateBase.h"
#include "Blueprint/UserWidget.h"
#include "../Common/MyCharacter.h"
#include "Net/UnrealNetWork.h"
#include "../Common/MyGameInstance.h"
#include "MyScoreMapPlayerController.h"



AMyScoreMapGameMode::AMyScoreMapGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PlayerControllerClass = AMyScoreMapPlayerController::StaticClass();
	GameStateClass = AMyScoreMapGameStateBase::StaticClass();
	DefaultPawnClass = AMyCharacter::StaticClass();


}

// Called when the game starts or when spawned
void AMyScoreMapGameMode::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* gameState = UGameplayStatics::GetGameState(GetWorld());
	_gameState = Cast<AMyScoreMapGameStateBase>(gameState);


}



void AMyScoreMapGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if ((Cast<UMyGameInstance>(GetGameInstance())->_gameStarted))
	{
		return;
	}
	else
	{
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


APlayerController* AMyScoreMapGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if ((Cast<UMyGameInstance>(GetGameInstance())->_gameStarted))
	{
		return NULL;
	}
	else
	{
		return Super::Login(NewPlayer, InRemoteRole, Options, Portal, UniqueId, ErrorMessage);
	}
}
