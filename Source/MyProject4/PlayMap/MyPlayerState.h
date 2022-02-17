// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../Common/MyGameInstance.h"
#include "MyPlayerState.generated.h"

/**
 *
 */
UCLASS()
class MYPROJECT4_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

protected:
	virtual void BeginPlay();


public:

	
	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		bool _IsSpectating;

	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		bool _IsFirst;

	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		TArray<class AMyPickups*> _Inventory;


	UPROPERTY(replicatedUsing = OnRep_InitializeColor)
	FCharacterInfo _characterInfo;


	UPROPERTY(replicated)
		bool _Initialized;

	UPROPERTY(replicated)
		bool _allMightyMode;


	UFUNCTION(Server, Reliable)
		void NotifyConnection();

	UFUNCTION(Server, Reliable)
		void OnRep_InitializeColor();

};
