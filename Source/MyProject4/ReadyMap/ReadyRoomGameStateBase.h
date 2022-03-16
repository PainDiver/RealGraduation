// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "../Interface/MyChattingGameStateBaseInterface.h"
#include "UObject/CoreOnlineFwd.h"
#include "ReadyRoomGameStateBase.generated.h"

/**
 * 
 */



UCLASS()
class MYPROJECT4_API AReadyRoomGameStateBase : public AGameStateBase, public IMyChattingGameStateBaseInterface
{
	GENERATED_BODY()
	
public:
	AReadyRoomGameStateBase();

protected:
	virtual void BeginPlay();


public:
	virtual void Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable, Server, Reliable)
		virtual void FindAllPlayerControllerHideAllWidget()override;

	UFUNCTION(NetMulticast, Reliable)
		virtual void HideAllWidget()override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
		virtual void FindAllPlayerControllerOpenAllWidget()override;

	UFUNCTION(NetMulticast, Reliable)
		virtual void OpenAllWidget()override;

	UFUNCTION(BlueprintCallable, Category = "User Capacity")
	virtual	int GetNumberOfUser()override { return PlayerArray.Num(); };

	UFUNCTION(BlueprintCallable)
	inline bool GetIsStartable() const{ return _bIsStartable; }

	UFUNCTION(BlueprintCallable)
		void SetIsStartable(bool value) { _bIsStartable = value; }

private:

	bool _bIsStartable;

};
