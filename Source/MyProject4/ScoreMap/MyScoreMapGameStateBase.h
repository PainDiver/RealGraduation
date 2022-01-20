// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "../Common/MyGameInstance.h"
#include "../Interface/MyChattingGameStateBaseInterface.h"
#include "MyScoreMapGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API AMyScoreMapGameStateBase : public AGameStateBase, public IMyChattingGameStateBaseInterface
{
	GENERATED_BODY()
public:
	AMyScoreMapGameStateBase();
protected:
	virtual void BeginPlay();

public:
	UFUNCTION(BlueprintCallable, Category = "User Capacity")
		virtual	int GetNumberOfUser()override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
		virtual void FindAllPlayerControllerHideAllWidget()override;

	UFUNCTION(NetMulticast, Reliable)
		virtual void HideAllWidget()override;



	UFUNCTION(BlueprintCallable, Server, Reliable)
		virtual void FindAllPlayerControllerOpenAllWidget()override;

	UFUNCTION(NetMulticast, Reliable)
		virtual void OpenAllWidget()override;

	UPROPERTY()
		uint8 _leastNum;

	uint8 _gameCount;

	bool _bstartable;


};
