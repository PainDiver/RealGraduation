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
	Cast<UMyGameInstance>(GetGameInstance())->_gameEnterClosed = false;

}



void AMyScoreMapGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
	if (!(_gameInstance->CheckOpenPublicConnection(true)))
	{
		return;
	}

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	
}


APlayerController* AMyScoreMapGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Options, Portal, UniqueId, ErrorMessage);
	
}

void AMyScoreMapGameMode::Logout(AController* Exiting)
{
	if (!Exiting->GetNetOwningPlayer())
	{
		_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
		_gameInstance->CheckOpenPublicConnection(false);
		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "exiting");
	}

	return Super::Logout(Exiting);
}