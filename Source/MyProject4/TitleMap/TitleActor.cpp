// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ATitleActor::ATitleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_staticMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ATitleActor::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(_timerHandle, this, &ATitleActor::JumpOnTime, time, true, 2.f);
}

void ATitleActor::JumpOnTime()
{
	if (HasAuthority())
	{
		_staticMesh->AddForce(FVector(0, 0, Force));
	}
}
