// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
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

	UFUNCTION(NetMulticast, Reliable)
	void NotifyFin_Net();

	UFUNCTION(Server, Reliable)
		void LetPlayerMove();

	UFUNCTION(NetMulticast, Reliable)
		void LetPlayerMove_Client();

	UPROPERTY(replicated,BlueprintReadOnly)
	float _finalTimer;

	UPROPERTY(replicated, BlueprintReadOnly)
		float _StartTimer;
};
