// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "../Common/MyGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "MyGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "../Common/MyCharacter.h"
#include "../ReadyMap/ReadyRoomGameStateBase.h"
#include "../Common/MySaveGame.h"

AMyPlayerState::AMyPlayerState()
{
	_IsSpectating = false;
	_IsFirst = false;
	_Initialized = false;
	_allMightyMode = false;
}


void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	auto controller = Cast<AController>(GetOwner());
	if (controller && controller->IsLocalController())
	{
		UMyGameInstance* localInstance = Cast<UMyGameInstance>(GetGameInstance());
		if (!localInstance)
		{
			return;
		}
		SetCharacterInfo_Server(localInstance->_characterInfo);
		NotifyConnection();
	}
	UE_LOG(LogTemp, Warning, TEXT("state Okay"));
}

void AMyPlayerState::SetCharacterInfo_Server_Implementation(const FCharacterInfo& info)
{
	_characterInfo = info;
	_characterInfo.OnRepToggle = !_characterInfo.OnRepToggle;
	_Initialized = true;
}



void AMyPlayerState::NotifyConnection_Implementation()
{
	
	UWorld* world = GetWorld();

	if (!world)
	{
		return;
	}

	AMyGameStateBase* gsb = world->GetGameState<AMyGameStateBase>();
	//this will not occur unless it's game map.
	if (GetWorld()->GetName().Equals("ServerMap"))
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyConnection Skipped"));
		return;
	}
	
	if (gsb)
	{
		gsb->AddConnectedPlayerInfo(_characterInfo);
	}
	
}


void AMyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, _IsSpectating);
	DOREPLIFETIME(AMyPlayerState, _IsFirst);
	DOREPLIFETIME(AMyPlayerState, _Inventory);
	DOREPLIFETIME(AMyPlayerState, _Initialized);
	DOREPLIFETIME(AMyPlayerState, _allMightyMode);
	DOREPLIFETIME(AMyPlayerState, _characterInfo);

}


