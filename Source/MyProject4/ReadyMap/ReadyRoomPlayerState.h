// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ReadyRoomPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API AReadyRoomPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AReadyRoomPlayerState();

protected:
	virtual void BeginPlay()override;
public:
	virtual void CopyProperties(APlayerState* playerstate);

	virtual void DispatchCopyProperties(APlayerState* playerstate);


};
