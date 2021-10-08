// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPickups.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMyPickups::AMyPickups()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	_effectTime = 2.f;
	_effect = 2.f;
}

void AMyPickups::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if (OtherActor)
	{
		AMyCharacter* target = Cast<AMyCharacter>(OtherActor);
		if (target)
		{
			_character = target;
			OnPickUp();
		}
	}
}

void AMyPickups::OnPickUp()
{
	switch (_ept)
	{
		case EPickUpType::EPT_Booster:
		{
			SpeedIncrease();
			break;
		}
		default:
			break;
	}
}

void AMyPickups::SpeedIncrease()
{
	
	_character->_characterMovementComponent->MaxWalkSpeed *= _effect;
	UE_LOG(LogTemp, Warning, TEXT("%f"), _character->_characterMovementComponent->MaxWalkSpeed);
	SetActorEnableCollision(false);
	_mesh->SetVisibility(false);
	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPickups::DestroySelf, 10.f, true, 10.f);
}

void AMyPickups::DestroySelf()
{
	switch (_ept)
	{
		case EPickUpType::EPT_Booster:
		{
			_character->_characterMovementComponent->MaxWalkSpeed /= _effect;
			break;
		}
		default:
			break;
	}
	GetWorldTimerManager().ClearTimer(_timerHandle);
	
	Destroy();
}