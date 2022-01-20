// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyRoomPlayerState.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "ReadyRoomPlayerController.h"


AReadyRoomPlayerState::AReadyRoomPlayerState()
{
}

void AReadyRoomPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AReadyRoomPlayerState::CopyProperties(APlayerState* playerstate)
{
	Super::CopyProperties(playerstate);
}


void AReadyRoomPlayerState::DispatchCopyProperties(APlayerState* playerstate)
{
	Super::DispatchCopyProperties(playerstate);
}
