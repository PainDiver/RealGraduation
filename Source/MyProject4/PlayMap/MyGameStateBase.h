// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "../Common/MyGameInstance.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */



UCLASS()
class MYPROJECT4_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AMyGameStateBase();

protected:
	virtual void BeginPlay();


public:
	
	UFUNCTION(Server,Reliable)
	void NotifyFin();

	UFUNCTION(Client, Reliable)
	void NotifyFin_Client();

	UFUNCTION(Server, Reliable)
		void LetPlayerMove();

	UFUNCTION(NetMulticast, Reliable)
		void LetPlayerMove_Client();

	UFUNCTION(Server, Reliable)
		void AddConnectedPlayerInfo(FCharacterInfo info);


	UPROPERTY(replicated,BlueprintReadOnly)
	float _finalTimer;

	UPROPERTY(replicated, BlueprintReadOnly)
		float _StartTimer;


	UPROPERTY(replicated, BlueprintReadOnly)
		bool _gameStarted;


	UPROPERTY(replicated, BlueprintReadOnly)
		TArray<FCharacterInfo> _connectedPlayersInfo;
};
