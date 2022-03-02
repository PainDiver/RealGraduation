// Fill out your copyright notice in the Description page of Project Settings.


#include "MySaveGame.h"
#include "Kismet/GamePlayStatics.h"
#include "GameFrameWork/Character.h"
#include "../Interface/MyStaticObjectSaveInterface.h"
#include "../Interface/MyInstantObjectSaveInterface.h"
#include "MyGameInstance.h"
#include "../PlayMap/MyGameStateBase.h"
#include "../PlayMap/MyProject4GameModeBase.h"
#include "../PlayMap/MyPlayerState.h"
#include "Engine/World.h"

UMySaveGame::UMySaveGame() 
{
}

void UMySaveGame::ClientLoad_Implementation(const UWorld* worldContext)
{	
	auto instance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0));
	if (!instance)
	{
		return;
	}
	RequestInitialLoadForMigration(instance->_playerData._transform, worldContext);
	UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);
	
}

void UMySaveGame::RequestInitialLoadForMigration_Implementation(const FTransform& transform, const UWorld* worldContext)
{
	auto character = UGameplayStatics::GetPlayerCharacter(worldContext, 0);
	character->SetActorTransform(transform);
}


void UMySaveGame::Save_Implementation(const UWorld* worldContext)
{
	auto instance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0));
	if (!instance)
	{
		instance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
	}
	
	SaveAllData(instance, worldContext);

	GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "Saved");
	UE_LOG(LogTemp, Warning, TEXT("saved"));

	UGameplayStatics::SaveGameToSlot(instance, HOST_MIGRATION, 0);
}


void UMySaveGame::ServerLoad_Implementation(UWorld* worldContext)
{

	auto instance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0));
	if (!instance)
	{
		return;
	}
	LoadAllData(instance, worldContext);
	UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);

}


void UMySaveGame::SaveAllData_Implementation(UMySaveGame* saveGame, const UWorld* worldContext)
{
	auto character = UGameplayStatics::GetPlayerCharacter(worldContext, 0);
	if (!character)
	{
		return;
	}
	saveGame->_playerData._transform = character->GetTransform();

	auto ps = Cast<AMyPlayerState>(character->GetPlayerState());
	if (!ps)
	{
		return;
	}
	saveGame->_playerData._itemNum = ps->_Inventory.Num();
	int i=0;
	for (const auto& pickup : ps->_Inventory)
	{
		if (pickup)
		{
			saveGame->_playerData._type[i] = pickup->_ePickUpType;
			i++;
		}
	}

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsWithInterface(worldContext, UMyStaticObjectSaveInterface::StaticClass(), actors);
	for (const auto& object : actors)
	{
		Cast<IMyStaticObjectSaveInterface>(object)->Execute_SaveData(object);
	}

	UGameplayStatics::GetAllActorsWithInterface(worldContext, UMyInstantObjectSaveInterface::StaticClass(), actors);
	for (const auto& object : actors)
	{
		Cast<IMyInstantObjectSaveInterface>(object)->Execute_SaveData(object);
	}

	UGameplayStatics::SaveGameToSlot(saveGame, HOST_MIGRATION, 0);
}


void UMySaveGame::LoadAllData_Implementation(UMySaveGame* saveGame, UWorld* worldContext)
{
	auto character = UGameplayStatics::GetPlayerCharacter(worldContext, 0);
	
	if (character)
	{
		character->SetActorTransform(saveGame->_playerData._transform);
	}
	auto ps = Cast<AMyPlayerState>(character->GetPlayerState());
	for (int i = 0; i < saveGame->_playerData._itemNum; i++)
	{
		auto item = worldContext->SpawnActor<AMyPickups>();
		item->SetOwner(character);
		item->_ePickUpType = saveGame->_playerData._type[i];
		item->OnActorBeginOverlap.Clear();
		ps->_Inventory.Add(item);
	}

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsWithInterface(worldContext, UMyStaticObjectSaveInterface::StaticClass(), actors);
	for (const auto& object : actors)
	{
		for (const auto& data : saveGame->_staticObjectData)
		{
			if (object->IsA(data._classType))
			{
				object->SetActorTransform(data._transform);
				saveGame->_staticObjectData.Remove(data);
				break;
			}
		}
	}

	UGameplayStatics::GetAllActorsWithInterface(worldContext, UMyInstantObjectSaveInterface::StaticClass(), actors);
	for (const auto& object : actors)
	{
		for (const auto& data : saveGame->_instantObjectData)
		{
			if (object->IsA(data._classType))
			{
				auto instantActor = GetWorld()->SpawnActor(data._classType);
				instantActor->SetActorTransform(data._transform);
				saveGame->_instantObjectData.Remove(data);
				break;
			}
		}
	}

	auto instance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(worldContext));
	auto state = UGameplayStatics::GetGameState(worldContext);
	auto mode = UGameplayStatics::GetGameMode(worldContext);

	if (state && mode)
	{
		auto gameState = Cast<AMyGameStateBase>(state);
		auto gameMode = Cast<AMyProject4GameModeBase>(mode);
		gameState->_bGameStarted = instance->GetMigrationInfo()._IsStarted;
		if (gameState->_bGameStarted)
		{
			gameMode->_StartTimer = 0;
		}
	}

}
