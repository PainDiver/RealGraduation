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
	move._interactInput = _interactInput;
	move._attackInput = _attackInput;
	move._greetInput = _greetInput;

	_jumpInput = false;
	_interactInput = false;
	_attackInput = false;
	_greetInput = false;

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
	Interact(MoveInfo._interactInput);
	Attack(MoveInfo._attackInput);
}

void UMyCharacterActionComponent::ReplayMove(const FCharacterMoveInfo& MoveInfo)
{
	if (!_owner.IsValid() || !_owner->_characterMovementComponent) return;

	_owner->_characterMovementComponent->SetComponentTickInterval(MoveInfo._deltaTime);
	AddMoveForward(MoveInfo._deltaTime, MoveInfo._forwardInput);
	AddMoveRight(MoveInfo._deltaTime, MoveInfo._rightInput);
	Jump(MoveInfo._jumpInput);
	Greet(MoveInfo._greetInput);
	Interact(MoveInfo._interactInput);
	Attack(MoveInfo._attackInput);
}


void UMyCharacterActionComponent::Interact(bool Input)
{
	if (Input && _interactDele.IsBound())
	{
		_interactDele.Broadcast(GetOwner());
		_interactDele.Clear();
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



