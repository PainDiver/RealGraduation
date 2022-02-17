// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "../Common/MyGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "MyGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "../Common/MyCharacter.h"

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

	if (HasLocalNetOwner())
	{
		DisableInput(GetOwner<APlayerController>());
		NotifyConnection();
	}

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
		gsb->_numOfConnectedPlayerInCurrentSession++;
	}
}

void AMyPlayerState::OnRep_InitializeColor_Implementation()
{
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyCharacter::StaticClass(), actors);
	
	for (const auto& player : actors)
	{
		AMyCharacter* character = Cast<AMyCharacter>(player);
		if (character)
		{
			character->SetColor(Cast<AMyPlayerState>(character->GetPlayerState())->_characterInfo._characterMesh);
		}
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
	DOREPLIFETIME(AMyPlayerState, _characterInfo)
}


