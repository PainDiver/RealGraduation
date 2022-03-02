// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyReadyRoomGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API AMyReadyRoomGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMyReadyRoomGameModeBase();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

	virtual void Logout(AController* Exiting);

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Capacity")
	//uint8 _maxNum;

	//uint8 _userNum;


	class AReadyRoomGameStateBase* _gameState;

	class UMyGameInstance* _gameInstance;


};
