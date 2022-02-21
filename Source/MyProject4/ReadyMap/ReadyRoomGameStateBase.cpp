// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyRoomGameStateBase.h"
#include "ReadyRoomPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "ReadyRoomPlayerState.h"
#include "../PlayMap/MyPlayerState.h"
#include "Net/UnrealNetwork.h"

const static uint8 MIN_PLAYER = 2;

AReadyRoomGameStateBase::AReadyRoomGameStateBase()
{
	PrimaryActorTick.bCanEverTick=true;
	_bIsStartable = false;
}

void AReadyRoomGameStateBase::BeginPlay()
{
	Super::BeginPlay();
}

void AReadyRoomGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PlayerArray.Num() >= MIN_PLAYER)
	{
		if (_bIsStartable == false)
			_bIsStartable = true;
	}
	else
	{
		if (_bIsStartable == true)
			_bIsStartable = false;
	}



}



void AReadyRoomGameStateBase::FindAllPlayerControllerHideAllWidget_Implementation()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), OutActors);

	for (auto playerController : OutActors)
	{
		AReadyRoomPlayerController* pc = Cast<AReadyRoomPlayerController>(playerController);

		pc->GetWorldTimerManager().ClearTimer(pc->_timerHandle);
		pc->GetWorldTimerManager().ClearTimer(pc->_timerHandle2);
	}
	HideAllWidget();
}

void AReadyRoomGameStateBase::HideAllWidget_Implementation()
{
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AReadyRoomPlayerController* pc = Cast<AReadyRoomPlayerController>(playerController);

	pc->GetWorldTimerManager().ClearTimer(pc->_timerHandle);
	pc->GetWorldTimerManager().ClearTimer(pc->_timerHandle2);
}



void AReadyRoomGameStateBase::FindAllPlayerControllerOpenAllWidget_Implementation()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), OutActors);
	for (auto playerController : OutActors)
	{
		AReadyRoomPlayerController* pc = Cast<AReadyRoomPlayerController>(playerController);
		if (pc->_ReadyHUDOverlay)
		{
			pc->_ReadyHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
		if (pc->_ChattingHUDOverlay)
		{
			pc->_ChattingHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}

		if (pc->_StartHUDOverlay)
		{
			pc->_StartHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
	OpenAllWidget();
}


void AReadyRoomGameStateBase::OpenAllWidget_Implementation()
{
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AReadyRoomPlayerController* pc = Cast<AReadyRoomPlayerController>(playerController);
	if (pc->_ReadyHUDOverlay)
	{
		pc->_ReadyHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	if (pc->_ChattingHUDOverlay)
	{
		pc->_ChattingHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	if (pc->_StartHUDOverlay)
	{
		pc->_StartHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}

	
}

