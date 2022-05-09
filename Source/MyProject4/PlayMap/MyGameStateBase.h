// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "../Common/MyGameInstance.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCountTimer
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _minutes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _seconds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float _milliseconds;

	void Count(const float& deltaTime)
	{
		_milliseconds += deltaTime;
		if (_milliseconds >= 1)
		{
			_seconds++;
			_milliseconds--;
		}
		if (_seconds >= 60)
		{
			_minutes++;
			_seconds = 0;
		}
		
	}

};

UCLASS()
class MYPROJECT4_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AMyGameStateBase();

protected:
	virtual void BeginPlay();


public:
	
	UFUNCTION(Server,Reliable,BlueprintCallable)
	void NotifyFin();

	
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
		bool _bIsAllPlayersReady;

	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bGameStarted;

	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bGameEnded;

	UPROPERTY(replicated, BlueprintReadOnly)
		TArray<FCharacterInfo> _connectedPlayersInfo;

	UPROPERTY(replicated, BlueprintReadOnly)
		FCountTimer _timer;

};
