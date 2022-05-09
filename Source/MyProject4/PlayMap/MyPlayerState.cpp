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

	auto controller = GetOwner<AController>();

	if (controller && controller->IsLocalPlayerController())
	{
		UMyGameInstance* localInstance = Cast<UMyGameInstance>(GetGameInstance());
		if (!localInstance)
		{
			return;
		}
		SetCharacterInfo_Server(localInstance->_characterInfo);
		auto character = Cast<AMyCharacter>(GetOwner());
		if (!character)
		{
			return;
		}
		character->SetColor(localInstance->_characterInfo._characterColor);
		
		//if (!localInstance->_IsDedicatedServer)
		//{
		//	localInstance->BindAltF4(true);
		//	localInstance->_migration._IsHost = false;
		//}
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

void AMyPlayerState::OnRep_InitializeColorAndNotifyConnection_Implementation()
{		
	if (GetWorld()->IsServer())
	{
		OnRep_InitializeColorAndNotifyConnection_Multi();
	}
}

void AMyPlayerState::OnRep_InitializeColorAndNotifyConnection_Multi_Implementation()
{
	auto character = Cast<AMyCharacter>(GetOwner());
	if (!character)
	{
		return;
	}
	
	auto ps = Cast<AMyPlayerState>(character->GetPlayerState());
	
	if (!ps)
	{
		return;
	}
	//character->SetColor(FColor::Blue);
	character->SetColor(ps->_characterInfo._characterColor);
	
}



void AMyPlayerState::NotifyConnection_Implementation()
{
	
	UWorld* world = GetWorld();

	if (!world)
	{
		return;
	}

	AMyGameStateBase* gsb = world->GetGameState<AMyGameStateBase>();
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


