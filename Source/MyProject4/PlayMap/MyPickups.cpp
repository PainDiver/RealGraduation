// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPickups.h"
#include "../Common/MyCharacter.h"
#include "MyWorldTimer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Common/MyGameInstance.h"
#include "MyPlayerState.h"
#include "Net/UnrealNetWork.h"
#include "../Common/MyCharacterActionComponent.h"
#include "Components/SphereComponent.h"
#include "../PlayMap/MyPickups.h"
#include "Camera/CameraComponent.h"

// Sets default values
AMyPickups::AMyPickups()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.	
	_effect = 250.f;
	
	_ePickUpType = _ePickUpType = static_cast<EPickUpType>(FMath::RandRange(0, static_cast<int32>(EPickUpType::EPT_MAX) - 2));

	static ConstructorHelpers::FClassFinder<AActor> grabProjectile(TEXT("/Game/BluePrintFrom_Me/Grab"));
	if (grabProjectile.Succeeded())
	{
		_grabProjectile = grabProjectile.Class;
	}

}

void AMyPickups::BeginPlay()
{
	Super::BeginPlay();

	if (_ePickUpType == EPickUpType::EPT_Random)
	{
		_ePickUpType = static_cast<EPickUpType>(FMath::RandRange(0, static_cast<int32>(EPickUpType::EPT_MAX) - 2));
	}
}

void AMyPickups::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	_character = Cast<AMyCharacter>(OtherActor);
	if (_character)
	{
		if (HasAuthority())
		{
			
			RPCAdd(_character);
		}
	}
	
}

void AMyPickups::RPCAdd_Implementation(AMyCharacter* character)
{
	_playerState = Cast<AMyPlayerState>(character->GetPlayerState());
	if (_playerState->_Inventory.Num() < 2)
	{
		SetActorEnableCollision(false);
		_mesh->SetVisibility(false);

		auto item = GetWorld()->SpawnActor<AMyPickups>();
		item->OnActorBeginOverlap.Clear();
		item->SetOwner(_character);
		item->_ePickUpType = _ePickUpType;
		
		_playerState->_Inventory.Add(item);
		FTimerHandle delay;
		GetWorldTimerManager().SetTimer(delay, FTimerDelegate::CreateLambda([&]()
			{
				_ePickUpType = static_cast<EPickUpType>(FMath::RandRange(0, static_cast<int32>(EPickUpType::EPT_MAX) - 2));
				SetActorEnableCollision(true);
				_mesh->SetVisibility(true);
			}
		), 10, false);
	}
}



void AMyPickups::Activate_Implementation()
{
	_character = GetOwner<AMyCharacter>();

	switch (_ePickUpType)
	{
	case EPickUpType::EPT_Booster:
	{
		
		_character->SetSpeed_Server(_effect);

		GetWorld()->GetTimerManager().SetTimer(_timerHandle, FTimerDelegate::CreateUObject(this, &AMyPickups::RestoreSelf), 10.f, false);
		UE_LOG(LogTemp, Warning, TEXT("%f"), _character->GetCharacterMovement()->MaxWalkSpeed);
		break;
	}
	case EPickUpType::EPT_Rocket:
	{
		_character->GetCharacterMovement()->AddImpulse(FVector(0, 0, 200000));
		GetWorld()->GetTimerManager().SetTimer(_timerHandle, FTimerDelegate::CreateUObject(this, &AMyPickups::RestoreSelf), 10.f, false);
		break;
	}
	case EPickUpType::EPT_Grab:
	{
		auto owner = GetOwner();
		auto character = Cast<AMyCharacter>(owner);
		auto newLocation = character->GetActorLocation()+FVector(0,0,0) + character->GetActorForwardVector()*150;
		auto newRotation = character->_camera->GetForwardVector().Rotation();
		
		AActor* projectile;
		if (_grabProjectile)
		{
			projectile = GetWorld()->SpawnActor(_grabProjectile, &newLocation, &newRotation);
			if (projectile)
			{
				projectile->SetReplicates(true);
				projectile->SetOwner(GetOwner());
				UE_LOG(LogTemp, Warning, TEXT("spawned!"));
			}
		}

	}
	default:
		break;
	}
}


void AMyPickups::RestoreSelf_Implementation()
{
	RestoreSelf_Multi();
}

void AMyPickups::RestoreSelf_Multi_Implementation()
{
	_character = GetOwner<AMyCharacter>();
	if (!_character)
	{
		return;
	}

	switch (_ePickUpType)
	{
	case EPickUpType::EPT_Booster:
	{
		_character->SetSpeed_Server(-_effect);
		break;
	}
	default:
		break;
	}
	GetWorldTimerManager().ClearTimer(_timerHandle);

	Destroy();
}
