// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "../Common/MyGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "MyGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "../Common/MyCharacter.h"
#include "../ReadyMap/ReadyRoomGameStateBase.h"

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




	if (HasNetOwner())
	{
		UMyGameInstance* localInstance = Cast<UMyGameInstance>(GetGameInstance());
		DisableInput(GetOwner<APlayerController>());
		if (localInstance->_characterInfo.initialized)
		{
			SetCharacterInfo_Server(localInstance->_characterInfo);
			if (IsNetMode(ENetMode::NM_ListenServer))
			{
				OnRep_InitializeColorAndNotifyConnection();
			}
		}
	
		UGameViewportClient* viewPortClient = localInstance->GetGameViewportClient();
		if (viewPortClient)
		{
			if(viewPortClient->OnWindowCloseRequested().IsBound())
			{
				viewPortClient->OnWindowCloseRequested().Unbind();
			}
			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool
				{
					UMyGameInstance* localInstance = Cast<UMyGameInstance>(GetGameInstance());
					AReadyRoomGameStateBase* gameState = Cast<AReadyRoomGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
					if (gameState)
					{
						gameState->FindAllPlayerControllerHideAllWidget();
					}
					if (localInstance)
					{
						localInstance->_exitRequest = true;
						localInstance->DestroySession();
					}
					return false;
				});
		}
	}
}

void AMyPlayerState::SetCharacterInfo_Server_Implementation(FCharacterInfo info)
{
	_characterInfo = info;
	_Initialized = true;
}

void AMyPlayerState::OnRep_InitializeColorAndNotifyConnection_Implementation()
{	
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyCharacter::StaticClass(), actors);
	
	for (const auto& player : actors)
	{
		AMyCharacter* character = Cast<AMyCharacter>(player);
		character->SetColor(Cast<AMyPlayerState>(character->GetPlayerState())->_characterInfo._characterColor);
		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, character->GetName());
	}
	

	NotifyConnection();
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

void AMyPlayerState::BuildSessionForMigration_Implementation()
{
	UMyGameInstance* gameInstance = Cast<UMyGameInstance>(GetGameInstance());
	
	gameInstance->GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.AddUObject(gameInstance, &UMyGameInstance::OnDestroySessionServerMigration);
	gameInstance->GetCurrentSessionInterface()->DestroySession(gameInstance->_currentSessionName);

}



void AMyPlayerState::ClientMigration_Implementation()
{
	UMyGameInstance* gameInstance = Cast<UMyGameInstance>(GetGameInstance());

	gameInstance->GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.AddUObject(gameInstance, &UMyGameInstance::OnDestroySessionClientMigration);
	gameInstance->GetCurrentSessionInterface()->DestroySession(gameInstance->_currentSessionName);

}

void AMyPlayerState::OnRep_ServerMigration_Implementation()
{
	if (_migrationInfo._IsHost)
	{
		UE_LOG(LogTemp, Warning, TEXT("asd"));
		BuildSessionForMigration();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("oo"));
		ClientMigration();
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


