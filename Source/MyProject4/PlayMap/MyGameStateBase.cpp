// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MyWorldTimer.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyProject4GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "../Common/MyCharacter.h"
#include "UMG.h"

AMyGameStateBase::AMyGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
	_finalTimer = 0;
	_StartTimer = 5;
	_bGameEnded = false;
}

void AMyGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMyGameStateBase::NotifyFin_Implementation()
{
	_bGameEnded = true;

	TArray<AActor*> controllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyWorldTimer::StaticClass(), controllers);
	
	TArray<AActor*> trivialActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), trivialActors);

	for (auto playerController : controllers)
	{
		AMyWorldTimer* pc = Cast<AMyWorldTimer>(playerController);
		pc->GetWorldTimerManager().ClearAllTimersForObject(pc);
	}
	for (auto actor : trivialActors)
	{
		actor->SetActorTickEnabled(false);
		GetWorldTimerManager().ClearAllTimersForObject(actor);
	}

	if (GetWorld()->IsServer())
	{
		auto gameMode = Cast<AMyProject4GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

		if (gameMode)
		{
			auto timers = gameMode->GetTickTimers();

			for (auto timer : timers)
			{
				GetWorldTimerManager().ClearTimer(timer);
			}
		}
	}
}



void AMyGameStateBase::LetPlayerMove_Implementation()
{
	for (auto ps : PlayerArray)
	{
		ps->GetOwner<APlayerController>()->SetInputMode(FInputModeGameOnly());
	}
	LetPlayerMove_Client();
}

void AMyGameStateBase::LetPlayerMove_Client_Implementation()
{
	for (auto ps : PlayerArray)
	{
		if(ps->GetOwner<APlayerController>())
			ps->GetOwner<APlayerController>()->SetInputMode(FInputModeGameOnly());
	}
}


void AMyGameStateBase::AddConnectedPlayerInfo_Implementation(FCharacterInfo info)
{
	_connectedPlayersInfo.Emplace(info);
	UE_LOG(LogTemp, Warning, TEXT("Connection!"));
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, _finalTimer);
	DOREPLIFETIME(AMyGameStateBase, _StartTimer);
	DOREPLIFETIME(AMyGameStateBase, _bIsAllPlayersReady);
	DOREPLIFETIME(AMyGameStateBase, _bGameStarted);
	DOREPLIFETIME(AMyGameStateBase, _connectedPlayersInfo);
	DOREPLIFETIME(AMyGameStateBase, _bGameEnded);
	DOREPLIFETIME(AMyGameStateBase, _timer);


}