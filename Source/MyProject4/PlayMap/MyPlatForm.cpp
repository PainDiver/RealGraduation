// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlatForm.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/GameStateBase.h"
#include "GameFrameWork/Character.h"

// Sets default values
AMyPlatForm::AMyPlatForm()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_platformType = EPlatFormType::EPF_RotatingPlatform;

	_boxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = _boxComponent;

	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_staticMesh->SetupAttachment(GetRootComponent());

	_toggle = false;
	_speed = 20.f;
	//_boxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//_boxComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	//_boxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//_boxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

}

// Called when the game starts or when spawned
void AMyPlatForm::BeginPlay()
{
	Super::BeginPlay(); 
	_dynamicMatInstance = _staticMesh->CreateAndSetMaterialInstanceDynamic(0);
	_dynamicMatInstance->GetMaterial()->GetVectorParameterValue(TEXT("Color"), _platFormColor);

	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPlatForm::TurnOnToggle, 5.f, true, 5.0f);
	auto character =Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (character)
	{
		SetOwner(character);
	}
}

// Called every frame
void AMyPlatForm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Transform(DeltaTime);
}

void AMyPlatForm::Transform_Implementation(float DeltaTime)
{
	Transform_Multi(DeltaTime);
}

void AMyPlatForm::Transform_Multi_Implementation(float DeltaTime)
{
	switch (_platformType)
	{
	case EPlatFormType::EPF_RotatingPlatform:
	{
		FRotator rotation = GetActorRotation();
		FRotator xRot = { 0,0,_speed };
		AddActorLocalRotation(xRot * DeltaTime);
		break;
	}

	case EPlatFormType::EPF_TranslatingPlatform:
	{
		FVector dx(_speed * DeltaTime, 0, 0);
		if (_toggle)
		{
			dx = -dx;
		}
		AddActorWorldOffset(dx);
		break;
	}

	case EPlatFormType::EPF_InvisiblePlatform:
	{
		if (_toggle)
		{
			_platFormColor.A += DeltaTime / (_speed / 5);

			if (_platFormColor.A > 0.2f)
			{
				SetActorEnableCollision(true);
			}
		}
		else
		{
			_platFormColor.A -= DeltaTime / (_speed / 5);

			if (_platFormColor.A < 0.2f)
			{
				SetActorEnableCollision(false);
			}
		}
		SetTransparency();

		break;
	}
	default:
		break;
	}
}


void AMyPlatForm::SetTransparency_Implementation()
{
	_dynamicMatInstance = _staticMesh->CreateAndSetMaterialInstanceDynamic(0);
	_dynamicMatInstance->SetVectorParameterValue(TEXT("Color"), _platFormColor);
}




void AMyPlatForm::TurnOnToggle_Implementation()
{
	if (_toggle)
		_toggle = false;
	else
		_toggle = true;

}




void AMyPlatForm::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlatForm, _platFormColor);
}