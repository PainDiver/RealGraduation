// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "../PlayMap/MyPickups.h"
#include "MySaveGame.generated.h"


/**
 * 
 */

#define HOST_MIGRATION "HostMigration"

USTRUCT(Atomic, BlueprintType)
struct FStaticObjectData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform _transform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> _classType;

	bool operator==(const FStaticObjectData& data)
	{
		if (!_transform.Equals(data._transform))
		{
			return false;
		}
		if (_classType != data._classType)
		{
			return false;
		}
		return true;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FInstantObjectData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform _transform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> _classType;


	bool operator==(const FInstantObjectData& data)
	{
		if (!_transform.Equals(data._transform))
		{
			return false;
		}
		if (_classType != data._classType)
		{
			return false;
		}
		return true;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FPlayerData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform _transform;

	UPROPERTY()
	EPickUpType _type[2];

	UPROPERTY()
	uint8 _itemNum;
};


UCLASS()
class MYPROJECT4_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	
	UMySaveGame();

	
	UFUNCTION(Client, Reliable)
	void ClientLoad(const UWorld* worldContext);

	UFUNCTION(Server, Reliable)
	void RequestInitialLoadForMigration(const FTransform& transform, const UWorld* worldContext);


	UFUNCTION(Client, Reliable)
	void Save(const UWorld* worldContext);

	UFUNCTION(Client, Reliable)
	void ServerLoad(UWorld* worldContext);


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString _saveSlot;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<FStaticObjectData> _staticObjectData;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TArray<FInstantObjectData> _instantObjectData;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	FPlayerData _playerData;

private:


	UFUNCTION(Server, Reliable)
		void SaveAllData(UMySaveGame* savegame, const UWorld* worldContext);

	UFUNCTION(Server, Reliable)
		void LoadAllData(UMySaveGame* savegame, UWorld* worldContext);


};
