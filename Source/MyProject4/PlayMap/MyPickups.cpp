// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPickups.h"
#include "../Common/MyCharacter.h"
#include "MyWorldTimer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Common/MyGameInstance.h"
#include "MyPlayerState.h"
#include "Net/UnrealNetWork.h"

// Sets default values
AMyPickups::AMyPickups()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.	
	_effect = 1.5f;

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
		SetActorEnableCollision(false);
		_mesh->SetVisibility(false);
		SetOwner(_character);
		
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
		UE_LOG(LogTemp, Warning, TEXT("Item got"));
		_playerState->_Inventory.Add(this);
	}

}


void AMyPickups::Activate_Implementation()
{
	_character = GetOwner<AMyCharacter>();

	switch (_ePickUpType)
	{
	case EPickUpType::EPT_Booster:
	{
		_character->GetCharacterMovement()->MaxWalkSpeed *= _effect;
		GetWorld()->GetTimerManager().SetTimer(_timerHandle, FTimerDelegate::CreateUObject(this, &AMyPickups::RestoreSelf), 10.f, false);
		break;
	}
	case EPickUpType::EPT_Rocket:
	{
		_character->GetCharacterMovement()->AddImpulse(FVector(0, 0, 200000));
		GetWorld()->GetTimerManager().SetTimer(_timerHandle, FTimerDelegate::CreateUObject(this, &AMyPickups::RestoreSelf), 10.f, false);
		break;
	}
	default:
		break;
	}
}

void AMyPickups::RestoreSelf_Implementation()
{
	_character = GetOwner<AMyCharacter>();

	switch (_ePickUpType)
	{
	case EPickUpType::EPT_Booster:
	{
		_character->GetCharacterMovement()->MaxWalkSpeed = 500.f;
		break;
	}
	default:
		break;
	}
	GetWorldTimerManager().ClearTimer(_timerHandle);
	Destroy();
}
