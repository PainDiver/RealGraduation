// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWorldTimer.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AMyWorldTimer::AMyWorldTimer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	_finalTimer = 20.f;
	_bossTimer = 60.f;
}

// Called when the game starts or when spawned
void AMyWorldTimer::BeginPlay()
{
	Super::BeginPlay();

	if (_BossTimeHUDAsset)
	{
		_BossTimeHUDOverlay = CreateWidget<UUserWidget>(this, _BossTimeHUDAsset);
		_BossTimeHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_BossTimeHUDOverlay->AddToViewport();
	}

}

// Called every frame
void AMyWorldTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CountTime(DeltaTime);
}

void AMyWorldTimer::CountTime(float DeltaTime)
{
	if (_bBoss)
	{
		_bossTimer -= DeltaTime;
		if (_bossTimer < 0)
		{
			_bossTimer = 0;
		}
	}
	if (_bIsFinal)
	{
		_finalTimer -= DeltaTime;
		if (_finalTimer < 0)
		{
			_finalTimer = 0;
		}
	}
	if (!_bPaused)
	{
		_worldTimer += DeltaTime;
	}
}

