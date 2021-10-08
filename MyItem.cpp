// Fill out your copyright notice in the Description page of Project Settings.

#include "MyItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AMyItem::AMyItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_collisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	RootComponent = _collisionVolume;
	
	_collisionVolume->SetSphereRadius(50.f);

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_mesh->SetupAttachment(GetRootComponent());

	_bRotate = false;
	_rotatingRate = 45.f;
}

// Called when the game starts or when spawned
void AMyItem::BeginPlay()
{
	Super::BeginPlay();
	_collisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AMyItem::OnOverlapBegin);
	_collisionVolume->OnComponentEndOverlap.AddDynamic(this, &AMyItem::OnOverlapEnd);

	
}

// Called every frame
void AMyItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_bRotate)
	{
		FRotator newRotation = GetActorRotation();
		newRotation.Yaw += DeltaTime * _rotatingRate;
		SetActorRotation(newRotation);
	}

}

void AMyItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AMyItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}