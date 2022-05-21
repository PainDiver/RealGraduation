// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlatForm.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/GameStateBase.h"
#include "GameFrameWork/Character.h"
#include "MyWorldTimer.h"
#include "MyGameStateBase.h"
#include "MyProject4GameModeBase.h"

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
	if (!_dynamicMatInstance)
	{
		return;
	}
	_dynamicMatInstance->GetMaterial()->GetVectorParameterValue(TEXT("Color"), _platFormColor);

	_gameState = Cast<AMyGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));

	if (HasAuthority())
	{
		GenerateTimer();
	}
}


void AMyPlatForm::GenerateTimer()
{
	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPlatForm::TurnOnToggle_Multi, 5.f, true, 5.0f);
	GetWorldTimerManager().SetTimer(_timerHandle2, FTimerDelegate::CreateLambda([&]()
		{
			Transform_Multi(0.01);
		}), 0.01, true);

	auto gameMode = Cast<AMyProject4GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gameMode)
	{
		gameMode->RegisterTickTimers(_timerHandle);
		gameMode->RegisterTickTimers(_timerHandle2);
	}
}


// Called every frame
void AMyPlatForm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMyPlatForm::Transform_Multi_Implementation(float delta)
{
	switch (_platformType)
	{
	case EPlatFormType::EPF_RotatingPlatform:
	{
		FRotator rotation = GetActorRotation();
		FRotator xRot = { 0,0,_speed };
		AddActorLocalRotation(xRot * delta);
		break;
	}

	case EPlatFormType::EPF_TranslatingPlatform01:
	{
		FVector dx(_speed * delta, 0, 0);

		if (_toggle)
		{
			dx = -dx;
		}
		AddActorWorldOffset(dx);
		break;
	}

	case EPlatFormType::EPF_TranslatingPlatform02:
	{
		FVector dy(0, _speed * delta, 0);

		if (_toggle)
		{
			dy = -dy;
		}
		AddActorWorldOffset(dy);
		break;
	}

	case EPlatFormType::EPF_TranslatingPlatform03:
	{
		FVector dz(0, 0, _speed * delta);

		if (_toggle)
		{
			dz = -dz;
		}
		AddActorWorldOffset(dz);
		break;
	}

	case EPlatFormType::EPF_InvisiblePlatform:
	{
		if (_toggle)
		{
			_platFormColor.A += delta / (_speed / 5);

			if (_platFormColor.A > 0.2f)
			{
				SetActorEnableCollision(true);
			}
		}
		else
		{
			_platFormColor.A -= delta / (_speed / 5);

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


void AMyPlatForm::TurnOnToggle_Multi_Implementation()
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