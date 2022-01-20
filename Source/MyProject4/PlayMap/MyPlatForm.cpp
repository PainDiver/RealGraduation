// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlatForm.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMyPlatForm::AMyPlatForm()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_platformType = EPlatFormType::EPF_RotatingPlatform;

	_BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = _BoxComponent;

	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_staticMesh->SetupAttachment(GetRootComponent());

	_toggle = false;
	_speed = 20.f;
	//_BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//_BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	//_BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//_BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

}

// Called when the game starts or when spawned
void AMyPlatForm::BeginPlay()
{
	Super::BeginPlay(); 
	_dynamicMatInstance = _staticMesh->CreateAndSetMaterialInstanceDynamic(0);
	_dynamicMatInstance->GetMaterial()->GetVectorParameterValue(TEXT("Color"), _platFormColor);

	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPlatForm::TurnOnToggle, 5.f, true, 5.0f);
}

// Called every frame
void AMyPlatForm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		Transform(DeltaTime);
	}
}

void AMyPlatForm::Transform_Implementation(float DeltaTime)
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
		FVector vec = GetActorLocation();
		FVector dx(_speed * DeltaTime, 0, 0);
		if (_toggle)
		{
			dx = -dx;
		}

		SetActorLocation(vec + dx);
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