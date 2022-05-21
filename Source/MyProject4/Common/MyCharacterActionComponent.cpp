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
	

	static ConstructorHelpers::FClassFinder<UCameraShakeBase> JumpShake(TEXT("/Game/BluePrintFrom_Me/CameraShake/JumpCamShake"));
	if (JumpShake.Succeeded())
	{
		_jumpShake = JumpShake.Class;
	}


	static ConstructorHelpers::FClassFinder<UCameraShakeBase> WalkShake(TEXT("/Game/BluePrintFrom_Me/CameraShake/WalkCamShake"));
	if (WalkShake.Succeeded())
	{
		_walkShake = WalkShake.Class;
	}


	static ConstructorHelpers::FClassFinder<UCameraShakeBase> SprintShake(TEXT("/Game/BluePrintFrom_Me/CameraShake/SprintCamShake"));
	if (SprintShake.Succeeded())
	{
		_sprintShake = SprintShake.Class;
	}


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

//FCharacterMoveInfo UMyCharacterActionComponent::CreateMove(const float& DeltaTime)
//{
//	FCharacterMoveInfo move;
//	move._deltaTime = DeltaTime;
//	move._timeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
//	move._forwardInput = _forwardInput;
//	move._rotatingInput = _rotatingInput;
//	move._rightInput = _rightInput;
//	move._lookUpInput = _lookUpInput;
//	move._jumpInput = _jumpInput;
//	move._interactInput = _interactInput;
//	move._attackInput = _attackInput;
//	move._greetInput = _greetInput;
//
//	_jumpInput = false;
//	_interactInput = false;
//	_attackInput = false;
//	_greetInput = false;
//	_interactInput = false;
//
//	return move;
//}


//void UMyCharacterActionComponent::SimulateMove(const FCharacterMoveInfo& MoveInfo)
//{
//
//	if (!_owner.IsValid() || !_owner->_characterMovementComponent) return ;
//	
//	auto movementComponent = _owner->GetMovementComponent();
//	if (!movementComponent)
//	{
//		return;
//	}
//	movementComponent->SetComponentTickInterval(MoveInfo._deltaTime);
//
//	AddMoveForward(MoveInfo._deltaTime, MoveInfo._forwardInput); //movement component won't behave in Server
//	AddMoveRight(MoveInfo._deltaTime, MoveInfo._rightInput);
//	AddLookUp(MoveInfo._deltaTime, MoveInfo._lookUpInput);
//	AddRotation(MoveInfo._deltaTime, MoveInfo._rotatingInput);
//	Jump(MoveInfo._jumpInput);
//	
//	Greet(MoveInfo._greetInput);
//	Attack(MoveInfo._attackInput);
//	Interact(MoveInfo._interactInput); //interact should be activated only through server(can affect non-self object)
//	
//}

//void UMyCharacterActionComponent::ReplayMove(const FCharacterMoveInfo& MoveInfo)
//{
//	if (!_owner.IsValid() || !_owner->_characterMovementComponent) return;
//
//	_owner->GetMovementComponent()->SetComponentTickInterval(MoveInfo._deltaTime);
//
//	// can't use custom dead Reckoning with character movement Component
//	
//	//AddMoveForward(MoveInfo._deltaTime, MoveInfo._forwardInput);
//	//AddMoveRight(MoveInfo._deltaTime, MoveInfo._rightInput);
//	//AddLookUp(MoveInfo._deltaTime, MoveInfo._lookUpInput);
//	//AddRotation(MoveInfo._deltaTime, MoveInfo._rotatingInput);
//	//Jump(MoveInfo._jumpInput); 
//	
//	Greet(MoveInfo._greetInput);
//	Attack(MoveInfo._attackInput);
//}


void UMyCharacterActionComponent::UseItem_Implementation()
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(_owner->GetPlayerState());
	
	if (!_owner || !playerState) return;
	if (playerState->_Inventory.Num() == 0) return;

	AMyPickups* item = playerState->_Inventory.Pop();

	if (item)
	{
		item->Activate();
	}

}


void UMyCharacterActionComponent::Interact(bool Input)
{
	if (Input && _interactDele.IsBound())
	{
		if (_owner)
		{
			BurstInteract();
		}
	}
	else if(Input)
	{
		if(_owner->IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("item used"));

			UseItem();
		}
	}
}


void UMyCharacterActionComponent::BurstInteract_Implementation()
{
	BurstInteract_Multi();
}

void UMyCharacterActionComponent::BurstInteract_Multi_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Ride Burst"));
	_interactDele.Broadcast(_owner);
	_interactDele.Clear();
}




void UMyCharacterActionComponent::Attack_Implementation(bool Input)
{
	if (!Input || !_owner || !_owner->_characterMovementComponent) return;
	
	if (_owner->_characterMovementComponent->IsFalling() || _bIsAttacking)
	{
		return;
	}
	_bIsAttacking = true;
}

void UMyCharacterActionComponent::Greet_Implementation(bool Input)
{
	if (!Input || !_owner || !_owner->_characterMovementComponent) return;

	if (_owner->_characterMovementComponent->IsFalling() || _bIsAttacking || _bIsGreeting)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("greeting"));
	_bIsGreeting = true;

	Greet_Multi();
}

void UMyCharacterActionComponent::Greet_Multi_Implementation()
{
	_owner->_characterMovementComponent->MaxWalkSpeed = 200;
}


void UMyCharacterActionComponent::AddMoveForward(const float& DeltaTime, const float& Input)
{
	if (!_owner) return;

	FRotator rotation = _owner->GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);

	_owner->AddMovementInput(direction, Input);

	//_owner->AddActorWorldOffset(DeltaTime * _owner->GetActorForwardVector() * 500 *Input);
}


void UMyCharacterActionComponent::AddMoveRight(const float& DeltaTime, const float& Input)
{
	if (!_owner) return;

	FRotator rotation = _owner->GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	_owner->AddMovementInput(direction, Input);

	//_owner->AddActorWorldOffset(DeltaTime * _owner->GetActorRightVector() * 500 * Input);
}


void UMyCharacterActionComponent::AddLookUp(const float& DeltaTime, const float& Input)
{
	if (!_owner) return;

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
	if (!_owner) return;

	_owner->AddControllerYawInput(Input);
}


void UMyCharacterActionComponent::SwimJump_Implementation()
{
	_owner->LaunchCharacter(FVector(0, 0, 600.f),false,true);
}

void UMyCharacterActionComponent::StartWalkCamShake(float scale)
{
	if (GetWorld()->GetFirstPlayerController())
	{
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(_walkShake, scale);
	}
}

void UMyCharacterActionComponent::StartJumpCamShake()
{
	if (GetWorld()->GetFirstPlayerController())
	{
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(_jumpShake, 1.0);
	}
}


void UMyCharacterActionComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyCharacterActionComponent, _bIsAttacking);
	DOREPLIFETIME(UMyCharacterActionComponent, _bIsGreeting);
}



