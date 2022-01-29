// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterActionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../PlayMap/MyPickups.h"
#include "Net/UnrealNetwork.h"
#include "../PlayMap/MyPlayerState.h"
#include "GameFrameWork/GameStateBase.h"


// Sets default values for this component's properties
UMyCharacterActionComponent::UMyCharacterActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
	_respawn = FVector(0.f, 0.f, 5.f);


	// ...
}


// Called when the game starts
void UMyCharacterActionComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	_owner = Cast<AMyCharacter>(GetOwner());
}


// Called every frame
void UMyCharacterActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FCharacterMoveInfo UMyCharacterActionComponent::CreateMove(const float& DeltaTime)
{
	FCharacterMoveInfo move;
	move._deltaTime = DeltaTime;
	move._timeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	move._forwardInput = _forwardInput;
	move._rotatingInput = _rotatingInput;
	move._rightInput = _rightInput;
	move._lookUpInput = _lookUpInput;
	move._jumpInput = _jumpInput;
	move._useInput = _useInput;
	move._attackInput = _attackInput;
	move._greetInput = _greetInput;
	move._rideInput = _rideInput;

	_jumpInput = false;
	_useInput = false;
	_attackInput = false;
	_greetInput = false;
	_rideInput = false;

	return move;
}


void UMyCharacterActionComponent::SimulateMove(const FCharacterMoveInfo& MoveInfo)
{
	if (!_owner.IsValid() || !_owner->_characterMovementComponent) return ;
	
	_owner->_characterMovementComponent->SetComponentTickInterval(MoveInfo._deltaTime);
	AddMoveForward(MoveInfo._deltaTime, MoveInfo._forwardInput);
	AddMoveRight(MoveInfo._deltaTime, MoveInfo._rightInput);
	AddLookUp(MoveInfo._deltaTime, MoveInfo._lookUpInput);
	AddRotation(MoveInfo._deltaTime, MoveInfo._rotatingInput);
	Jump(MoveInfo._jumpInput);
	Greet(MoveInfo._greetInput);
	UseItem(MoveInfo._useInput);
	Attack(MoveInfo._attackInput);
	Ride(_owner->_target, MoveInfo._rideInput);
}

void UMyCharacterActionComponent::ReplayMove(const FCharacterMoveInfo& MoveInfo)
{
	if (!_owner.IsValid() || !_owner->_characterMovementComponent) return;

	_owner->_characterMovementComponent->SetComponentTickInterval(MoveInfo._deltaTime);
	AddMoveForward(MoveInfo._deltaTime, MoveInfo._forwardInput);
	AddMoveRight(MoveInfo._deltaTime, MoveInfo._rightInput);
	Jump(MoveInfo._jumpInput);
	Greet(MoveInfo._greetInput);
	UseItem(MoveInfo._useInput);
	Attack(MoveInfo._attackInput);
	Ride(_owner->_target, MoveInfo._rideInput);
}


void UMyCharacterActionComponent::RespawnCheck()
{
	if (_owner->GetActorLocation().Z < -2000)
	{
		_owner->SetActorLocation(_respawn);
	}
}

void UMyCharacterActionComponent::ObjectScan_Implementation()
{
	if (!(_owner->_playerState && _owner->_playerState->_allMightyMode == true))
	{
		return;
	}

	FHitResult hit;
	//DrawDebugLine(GetWorld(), _camera->GetComponentLocation()+ _camera->GetComponentLocation().ForwardVector, GetControlRotation().Vector() * 10000,FColor::Red,false,-1.0f,0,2.f);
	if (_owner->GetWorld()->LineTraceSingleByChannel(hit, _owner->_camera->GetComponentLocation() + _owner->_camera->GetComponentLocation().ForwardVector, _owner->GetControlRotation().Vector() * 7, ECollisionChannel::ECC_Visibility))
	{
		if (_owner->_target != hit.GetActor())
		{
			_owner->_target = hit.GetActor();
		}
	}
	else
	{
		_owner->_target = nullptr;
	}
}


void UMyCharacterActionComponent::Ride_Implementation(AActor* target, bool Input)
{
	if (!Input) return;

	if (_owner->_target != target)
	{
		_owner->_target = target;
		if (_owner->_target && !_owner->_target->IsHidden())
			_owner->_target->SetActorHiddenInGame(true);
	}
	else
	{
		return;
	}

	/*
	RidableObject* ride = Cast<RidableObject>(target);
	if (ride)
	{
		possess(ride)
	}
	*/

}


void UMyCharacterActionComponent::UseItem(bool Input)
{
	if (!Input || !_owner.IsValid() || !_owner->_playerState) return;
	
	if (_owner->_playerState->_Inventory.Num() == 0) return;

	AMyPickups* item = _owner->_playerState->_Inventory.Pop();

	if (item)
	{
		item->Activate();
	}

}


void UMyCharacterActionComponent::Attack(bool Input)
{
	if (!Input || !_owner.IsValid() || !_owner->_characterMovementComponent) return;
	
	if (_owner->_characterMovementComponent->IsFalling() || _bIsAttacking)
	{
		return;
	}

	_owner->_characterMovementComponent->MaxWalkSpeed = 200;
	_bIsAttacking = true;

}

void UMyCharacterActionComponent::Greet(bool Input)
{
	if (!Input || !_owner.IsValid() || !_owner->_characterMovementComponent) return;

	if (_owner->_characterMovementComponent->IsFalling() || _bIsAttacking || _bIsGreeting)
	{
		return;
	}
	_owner->_characterMovementComponent->MaxWalkSpeed = 200;

	UE_LOG(LogTemp, Warning, TEXT("greeting"));
	_bIsGreeting = true;

}


void UMyCharacterActionComponent::AddMoveForward(const float& DeltaTime, const float& Input)
{
	if (!_owner.IsValid()) return;

	FRotator rotation = _owner->GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	
	
	_owner->AddMovementInput(direction, Input);
	
	//_owner->AddActorWorldOffset(DeltaTime * _owner->GetActorForwardVector() * 500 *Input);

}

void UMyCharacterActionComponent::AddMoveRight(const float& DeltaTime, const float& Input)
{
	if (!_owner.IsValid()) return;

	FRotator rotation = _owner->GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	_owner->AddMovementInput(direction, Input);

	//_owner->AddActorWorldOffset(DeltaTime * _owner->GetActorRightVector() * 500 * Input);
}

void UMyCharacterActionComponent::AddLookUp(const float& DeltaTime, const float& Input)
{
	if (!_owner.IsValid()) return;

	FRotator baseAimRotator = _owner->GetBaseAimRotation();
	if (Input >= 0)
	{
		if (baseAimRotator.Pitch > -65.f)
		{
			_owner->AddControllerPitchInput(Input);
		}
	}
	else
	{
		if (baseAimRotator.Pitch < 45.f)
		{
			_owner->AddControllerPitchInput(Input);
		}
	}



}

void UMyCharacterActionComponent::AddRotation(const float& DeltaTime, const float& Input)
{
	if (!_owner.IsValid()) return;

	_owner->AddControllerYawInput(Input);
}

void UMyCharacterActionComponent::Jump(const bool& Input)
{
	if (!Input || !_owner.IsValid()) return;

	_owner->Jump();

}


void UMyCharacterActionComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyCharacterActionComponent, _bIsAttacking);
	DOREPLIFETIME(UMyCharacterActionComponent, _bIsGreeting);
}



