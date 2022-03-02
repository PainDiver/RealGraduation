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
		SetCharacterInfo_Server(localInstance->_characterInfo);
		auto character = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		character->SetColor(localInstance->_characterInfo._characterColor);
		localInstance->BindAltF4(true);
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
	auto character = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	auto ps = Cast<AMyPlayerState>(character->GetPlayerState());
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


void AMyPlayerState::OnRep_ServerMigration_Implementation()
{
	if (GetOwner<APlayerController>() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->SetMigartionInfo(GetMigrationInfo());
		SaveBeforeExit();
	}
	else
	{
		auto state = Cast<AMyGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
		Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->SetMigartionInfo({ false,GetPlayerName(),GetWorld()->GetMapName(),state->_bGameStarted});
		SaveBeforeExit();
	}
}

void AMyPlayerState::SetMigrationInfo_Implementation(const FMigrationPacket& info)
{	
	GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, GetName());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *GetName());
	GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, FString::FromInt(static_cast<int>(GetLocalRole()))+" role");
	UE_LOG(LogTemp, Warning, TEXT("%d role"), static_cast<int>(GetLocalRole()));

	_migrationInfo = info;
}

void AMyPlayerState::SaveBeforeExit_Implementation()
{
	if (GetWorld()->IsServer())
	{
		return;
	}

	_saveGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0));
	if (!_saveGameInstance)
	{
		_saveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
	}
	_saveGameInstance->Save(GetWorld());
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
	DOREPLIFETIME(AMyPlayerState, _migrationInfo);

}


