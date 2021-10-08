// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWorldTimer.h"
#include "MyEnemy.h"
#include "Blueprint/UserWidget.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacter.h"

// Sets default values
AMyWorldTimer::AMyWorldTimer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	_finalTimer = 20.f;
	_bossTimer = 20.f;
	_bBoss = true;
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
	_character = Cast<AMyCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyWorldTimer::SpawnBoss, _bossTimer, true, _bossTimer);
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

void AMyWorldTimer::SpawnBoss()
{
	if (_enemyAsset)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FRotator rotator;
		FVector spawnLocation;
		spawnLocation.X = 500;
		spawnLocation.Y = 500;
		spawnLocation.Z += 90;


		AActor* enemyClass = GetWorld()->SpawnActor<AActor>(_enemyAsset, SpawnLocation, FRotator(0.f), SpawnParams);
		_enemy = Cast<AMyEnemy>(enemyClass);

		if (_enemy)
		{
			_enemy->SpawnDefaultController();
			AAIController* AICon = Cast<AAIController>(_enemy->GetController());
			if (AICon)
			{
				_enemy->_AIController = AICon;
			}
		}
		
		_bBoss = false;
		GetWorldTimerManager().PauseTimer(_timerHandle);
		GetWorldTimerManager().ClearTimer(_timerHandle);
	}
}