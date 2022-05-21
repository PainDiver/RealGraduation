// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterParkourComponent.h"
#include "MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UMyCharacterParkourComponent::UMyCharacterParkourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	_sprintSpeed = 250.f;
	_stamina = 100.f;
	_fuel = 100.f;
	// ...
}


// Called when the game starts
void UMyCharacterParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	_character = GetOwner<AMyCharacter>();
	if (_character)
	{
		_movementComponent = _character->GetCharacterMovement();

		_defaultCharacterSpeed = _movementComponent->MaxWalkSpeed;

		_camera = Cast<UCameraComponent>(_character->GetComponentByClass(UCameraComponent::StaticClass()));

		_capsuleComponent = _character->GetCapsuleComponent();

		_springArm = _character->_springArm;
	}
}

void UMyCharacterParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UMyCharacterParkourComponent::WallRunningServerCheck_Implementation(bool onOff)
{
	_bIsWallRunning = onOff;

}

void UMyCharacterParkourComponent::WallRunRight_Implementation()
{
	WallRunRight_Multi();
}

void UMyCharacterParkourComponent::WallRunRight_Multi_Implementation()
{
	if (!(_camera && _springArm && _capsuleComponent && _character && _movementComponent))
	{
		return;
	}

	auto characterLoc = _character->GetActorLocation();
	auto characterRightVector = _character->GetActorRightVector() * 150.f;
	auto characterForwardVector = _character->GetActorForwardVector() * -25.f;

	auto rayEnd = characterLoc + characterRightVector + characterForwardVector;

	FHitResult hitResult;
	bool firstWallCheck = GetWorld()->LineTraceSingleByChannel(hitResult, characterLoc, rayEnd, ECollisionChannel::ECC_Visibility);
	//DrawDebugLine(GetWorld(), characterLoc, rayEnd, FColor(255, 0, 0), false, 10.f);

	if (!firstWallCheck)
	{
		return;
	}

	auto previousVelocity = _movementComponent->Velocity;
	_movementComponent->Velocity = { previousVelocity.X, previousVelocity.Y, 0 };
	_wallNormal = hitResult.ImpactNormal;
	_movementComponent->GravityScale = 0.05f;

	//_character->SetReplicateMovement(false);
	
	WallRunningServerCheck(true);


		GetWorld()->GetTimerManager().SetTimer(_wallRunningRightTimer, FTimerDelegate::CreateLambda(
			[&]()
			{
				if (!_bIsWallRunning)
				{
					GetWorld()->GetTimerManager().ClearTimer(_wallRunningRightTimer);
				}
				else
				{
					auto characterLoc = _character->GetActorLocation();
					auto characterRightVector = _character->GetActorRightVector() * 150.f;
					auto characterForwardVector = _character->GetActorForwardVector() * -25.f;
					auto characterUpVector = _character->GetActorUpVector() * -100.f;

					auto rayEnd = characterLoc + characterRightVector + characterForwardVector + characterUpVector;

					FHitResult hitResult;
					bool wallCheck = GetWorld()->LineTraceSingleByChannel(hitResult, characterLoc, rayEnd, ECollisionChannel::ECC_Visibility);
					//DrawDebugLine(GetWorld(), characterLoc, rayEnd, FColor(255, 0, 0), false, 10.f);
					
					/*if (_character->IsLocallyControlled())
					{
						_character->CorrectLocation_Server(_character->GetActorLocation());
					}*/

					if (!wallCheck || !_movementComponent->IsFalling() || _movementComponent->Velocity.Size() < 300.f)
					{
						if (_character->IsLocallyControlled())
						{
							StopWallRunning(_character->GetActorLocation());
						}
						GetWorld()->GetTimerManager().ClearTimer(_wallRunningRightTimer);
					}
					else
					{
						_wallNormal = hitResult.ImpactNormal;
					}
				}
			}
		), 0.2f, true);
	

	_character->GetMesh()->SetRelativeRotation(FRotator(0, 0, -30));

	//if (_character->HasAuthority())
	//{
	//	auto newRot = _capsuleComponent->GetRelativeRotation();
	//	newRot.Roll = -60.0f;
	//	FLatentActionInfo LatentInfo;
	//	LatentInfo.CallbackTarget = this;
	//	UKismetSystemLibrary::MoveComponentTo(_capsuleComponent, _capsuleComponent->GetRelativeLocation(), newRot, false, false, 0.05f, false, EMoveComponentAction::Move, LatentInfo);
	//}


	if (_character->IsLocallyControlled())
	{
		auto Rot = _camera->GetRelativeRotation();
		Rot.Roll = -30.0f;
		FLatentActionInfo LatentInfo3;
		LatentInfo3.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(_camera, _camera->GetRelativeLocation(), Rot, true, true, 0.0f, false, EMoveComponentAction::Move, LatentInfo3);
	}

}

void UMyCharacterParkourComponent::WallRunLeft_Implementation()
{
	WallRunLeft_Multi();
}

void UMyCharacterParkourComponent::WallRunLeft_Multi_Implementation()
{
	if (!(_camera && _springArm && _capsuleComponent && _character && _movementComponent))
	{
		return;
	}

	auto characterLoc = _character->GetActorLocation();
	auto characterRightVector = _character->GetActorRightVector() * -150.f;
	auto characterForwardVector = _character->GetActorForwardVector() * -25.f;

	auto rayEnd = characterLoc + characterRightVector + characterForwardVector;


	FHitResult hitResult;
	bool wallCheck = GetWorld()->LineTraceSingleByChannel(hitResult, characterLoc, rayEnd, ECollisionChannel::ECC_Visibility);
	if (!wallCheck)
	{
		return;
	}
	
	auto previousVelocity = _movementComponent->Velocity;
	_movementComponent->Velocity = { previousVelocity.X, previousVelocity.Y, 0 };
	_wallNormal = hitResult.ImpactNormal;
	_movementComponent->GravityScale = 0.05f;

	//_character->SetReplicateMovement(false);
	WallRunningServerCheck(true);


		GetWorld()->GetTimerManager().SetTimer(_wallRunningRightTimer, FTimerDelegate::CreateLambda(
			[&]()
			{
				if (!_bIsWallRunning)
				{
					GetWorld()->GetTimerManager().ClearTimer(_wallRunningRightTimer);
				}
				else
				{
					auto characterLoc = _character->GetActorLocation();
					auto characterRightVector = _character->GetActorRightVector() * -150.f;
					auto characterForwardVector = _character->GetActorForwardVector() * -25.f;
					auto characterUpVector = _character->GetActorUpVector() * -100.f;

					auto rayEnd = characterLoc + characterRightVector + characterForwardVector + characterUpVector;

					/*if (_character->IsLocallyControlled())
					{
						_character->CorrectLocation_Server(_character->GetActorLocation());
					}*/

					FHitResult hitResult;
					bool wallCheck = GetWorld()->LineTraceSingleByChannel(hitResult, characterLoc, rayEnd, ECollisionChannel::ECC_Visibility);
					if (!wallCheck || !_movementComponent->IsFalling() || _movementComponent->Velocity.Size() < 300.f)
					{
						if (_character->IsLocallyControlled())
						{
							StopWallRunning(_character->GetActorLocation());
						}
						GetWorld()->GetTimerManager().ClearTimer(_wallRunningRightTimer);
					}
					else
					{
						_wallNormal = hitResult.ImpactNormal;
					}
				}
			}
		), 0.2f, true);

	
	_character->GetMesh()->SetRelativeRotation(FRotator(0, 0, 30));

	//if (_character->HasAuthority())
	//{
	//	auto newRot = _character->GetCapsuleComponent()->GetRelativeRotation();
	//	newRot.Roll = 60.0f;
	//	FLatentActionInfo LatentInfo;
	//	LatentInfo.CallbackTarget = this;
	//	UKismetSystemLibrary::MoveComponentTo(_capsuleComponent, _capsuleComponent->GetRelativeLocation(), newRot, false, false, 0.05f, true, EMoveComponentAction::Move, LatentInfo);
	//}

	if (_character->IsLocallyControlled())
	{
		FLatentActionInfo LatentInfo2;
		LatentInfo2.CallbackTarget = this;
		auto Rot = _camera->GetRelativeRotation();
		Rot.Roll = 30.0f;
		UKismetSystemLibrary::MoveComponentTo(_camera, _camera->GetRelativeLocation(), Rot, true, true, 0.0f, false, EMoveComponentAction::Move, LatentInfo2);
	}
}

void UMyCharacterParkourComponent::StopWallRunning_Implementation(const FVector& loc)
{
	//_character->SetActorLocation(loc);
	StopWallRunning_Multi();
	_bIsWallRunning = false;
}

void UMyCharacterParkourComponent::StopWallRunning_Multi_Implementation()
{
	_movementComponent->GravityScale = 1.0f;

	_character->GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);
	//_character->SetReplicateMovement(true);

	if (_character->HasAuthority())
	{
		FLatentActionInfo LatentInfo2;
		LatentInfo2.CallbackTarget = this;
		auto Rot = _capsuleComponent->GetRelativeRotation();
		Rot.Roll = 0.0f;
		UKismetSystemLibrary::MoveComponentTo(_capsuleComponent, _capsuleComponent->GetRelativeLocation(), Rot, false, false, 0.0f, true, EMoveComponentAction::Move, LatentInfo2);
	}

	if (_character->IsLocallyControlled())
	{
		//_character->DisableInput(Cast<APlayerController>(_character->GetController()));
		auto newRot = _camera->GetRelativeRotation();
		newRot.Roll = 0.0f;
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(_camera, _camera->GetRelativeLocation(), newRot, true, true, 0.1f, true, EMoveComponentAction::Move, LatentInfo);
		//_character->EnableInput(Cast<APlayerController>(_character->GetController()));
	}
}

void UMyCharacterParkourComponent::WallRunningJump_Implementation(const FVector& loc)
{
	_movementComponent->Launch(_wallNormal * 500 + FVector{ 0,0,800.f });
}


void UMyCharacterParkourComponent::LedgingServerCheck_Implementation(bool onOff)
{
	_bIsledging = onOff;
}

void UMyCharacterParkourComponent::Ledge_Implementation()
{
	Ledge_Multi();
}

void UMyCharacterParkourComponent::Ledge_Multi_Implementation()
{

	FHitResult hit;
	FVector start = _character->GetActorLocation() - _character->GetActorForwardVector() * -10.f;
	FVector end = _character->GetActorLocation() + _character->GetActorForwardVector() * 100 ;
	bool wallCheck = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility);

	if (!wallCheck)
	{
		return;
	}

	if (_character->IsLocallyControlled())
	{
		FLatentActionInfo latent;
		latent.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(_springArm, FVector(-800, 0, 0), FRotator(0, 0, 0), true, true, 0.2f, false, EMoveComponentAction::Move, latent);
	}

	auto newRot = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, -hit.ImpactNormal);
	if (_character->Controller)
	{
		_character->Controller->ClientSetRotation(FRotator(0, newRot.Yaw, 0));
	}

	_movementComponent->Velocity = FVector::ZeroVector;
	_movementComponent->GravityScale = 0.0f;	
	_capsuleComponent->SetWorldLocation(hit.Location + _character->GetActorForwardVector() * -10.f);

	if (_character->IsLocallyControlled())
	{
		_character->SetReplicateMovement(false);
	}

	if (_character->IsLocallyControlled())
	{
		LedgingServerCheck(true);
		GetWorld()->GetTimerManager().SetTimer(_ledgeTimer, FTimerDelegate::CreateLambda([&]()
			{
				if (_bIsledging)
				{
					FHitResult hit;
					FVector start = _character->GetActorLocation() + _character->GetActorForwardVector() * -10.f;
					FVector end = _character->GetActorLocation() + _character->GetActorForwardVector() * 100;
					bool frontCheck = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECollisionChannel::ECC_Visibility);
					
					FHitResult upHit;
					FVector upStart = _character->GetActorLocation();
					FVector upEnd = _character->GetActorLocation() + _character->GetActorUpVector() * 150 + _character->GetActorForwardVector() * 200;
					_upWall = GetWorld()->LineTraceSingleByChannel(upHit, upStart, upEnd, ECollisionChannel::ECC_Visibility);

					FHitResult downHit;
					FVector downStart = _character->GetActorLocation();
					FVector downEnd = _character->GetActorLocation() + _character->GetActorUpVector() * -100 + _character->GetActorForwardVector() *200;
					_downWall = GetWorld()->LineTraceSingleByChannel(downHit, downStart, downEnd, ECollisionChannel::ECC_Visibility);

					FHitResult leftHit;
					FVector leftStart = _character->GetActorLocation();
					FVector leftEnd = _character->GetActorLocation() + _character->GetActorRightVector() * -100 + _character->GetActorForwardVector() * 200;
					_leftWall = GetWorld()->LineTraceSingleByChannel(leftHit, leftStart, leftEnd, ECollisionChannel::ECC_Visibility);

					FHitResult rightHit;
					FVector rightStart = _character->GetActorLocation();
					FVector rightEnd = _character->GetActorLocation() + _character->GetActorRightVector() * 100 + _character->GetActorForwardVector() * 200;
					_rightWall = GetWorld()->LineTraceSingleByChannel(rightHit, rightStart, rightEnd, ECollisionChannel::ECC_Visibility);
				
					FHitResult feetHit;
					FVector feetStart = _character->GetActorLocation() + _character->GetActorUpVector() * -30.f;
					FVector feetEnd = _character->GetActorLocation() + _character->GetActorUpVector() * -100 + _character->GetActorForwardVector() * -100;
					_feetWall = GetWorld()->LineTraceSingleByChannel(feetHit, feetStart, feetEnd, ECollisionChannel::ECC_Visibility);

					_ledgeRightMoveDir = UKismetMathLibrary::Cross_VectorVector(rightHit.ImpactNormal, _character->GetActorUpVector());
					_ledgeRightMoveDir.Normalize();

					_ledgeUpMoveDir = UKismetMathLibrary::Cross_VectorVector(_character->GetActorRightVector(), upHit.ImpactNormal);
					_ledgeUpMoveDir.Normalize();

					_ledgeLeftMoveDir = UKismetMathLibrary::Cross_VectorVector(leftHit.ImpactNormal, -_character->GetActorUpVector());
					_ledgeLeftMoveDir.Normalize();

					_ledgeDownMoveDir = UKismetMathLibrary::Cross_VectorVector(-_character->GetActorRightVector(), downHit.ImpactNormal);
					_ledgeDownMoveDir.Normalize();

					if (!frontCheck  || _movementComponent->Velocity.Size()>20.f)
					{
						_upWall = false;
						_downWall = false;
						_leftWall = false;
						_rightWall = false;
						_feetWall = false;
						UE_LOG(LogTemp, Warning, TEXT("No Wall In Front"));
						_character->CorrectLocation_Server(_character->GetActorLocation());
						Unledge();
						GetWorld()->GetTimerManager().ClearTimer(_ledgeTimer);
					}
					else
					{
						
						_upRot = UKismetMathLibrary::FindLookAtRotation(_character->GetActorLocation(), upHit.Location);
						_upRot.Pitch -= 90.f;
						_upRot.Roll = 0;
						_upRot.Yaw = 0;

						if (_upRot.Pitch  < 0.0f)
						{
							_upRot.Pitch = 0.0f;
						}
						_upRot += _character->GetMesh()->GetRelativeRotation();
					}
				}}),
			0.05f, true);
	}
}

void UMyCharacterParkourComponent::LedgeJump_Implementation()
{
	Unledge();
	LedgeJump_Multi();
}

void UMyCharacterParkourComponent::LedgeJump_Multi_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(_ledgeTimer);
	_movementComponent->GravityScale = 1.0f;
	_movementComponent->Launch(FVector(0, 0, 600));
}

void UMyCharacterParkourComponent::Unledge_Implementation()
{
	Unledge_Multi();
}

void UMyCharacterParkourComponent::Unledge_Multi_Implementation()
{
	if (_bIsledging)
	{
		if (_character->IsLocallyControlled())
		{
			LedgingServerCheck(false);
			_character->SetReplicateMovement(true);
		}
		_capsuleComponent->SetRelativeRotation(FRotator(0, 0, 0));

		_character->bUseControllerRotationYaw = true;

		_movementComponent->GravityScale = 1.0f;
		
		if (_character->IsLocallyControlled())
		{
			FLatentActionInfo latent;
			latent.CallbackTarget = this;
			UKismetSystemLibrary::MoveComponentTo(_springArm,FVector(0,0,50),FRotator(0,0,0), true, true, 0.2f,false,EMoveComponentAction::Move,latent);
		}
	}
}

void UMyCharacterParkourComponent::Sprint_Implementation()
{
	if (!_bIsSprinting)
	{
		_bIsSprinting = true;
		Sprint_Multi();
	}
}

void UMyCharacterParkourComponent::Sprint_Multi_Implementation()
{
	_movementComponent->MaxWalkSpeed += _sprintSpeed;
}

void UMyCharacterParkourComponent::StopSprint_Implementation()
{
	if (_bIsSprinting)
	{
		_bIsSprinting = false;
		StopSprint_Multi();
	}
}


void UMyCharacterParkourComponent::StopSprint_Multi_Implementation()
{
	if (_defaultCharacterSpeed != _movementComponent->MaxWalkSpeed)
	{
		_movementComponent->MaxWalkSpeed -= _sprintSpeed;
	}

}

void UMyCharacterParkourComponent::CheckGage(float deltaTime)
{
	if (_bIsSprinting)
	{
		_stamina -= deltaTime * 15.f;
		if (_stamina < 0.f)
		{
			_regenerateDelay = true;
			StopSprint();
			_stamina = 0.f;
			FTimerHandle RegenDelay;
			GetWorld()->GetTimerManager().SetTimer(RegenDelay, FTimerDelegate::CreateLambda(
				[&]() 
				{
					_regenerateDelay = false;
				}),
				5.0f,false);
		}
	}
	if (_bIsGliding)
	{
		_fuel -= deltaTime * 20.f;
		if (_fuel < 0.f)
		{
			_fuelRegenerateDelay = true;
			UnGlide();
			_fuel = 0.f;
			FTimerHandle RegenDelay;
			GetWorld()->GetTimerManager().SetTimer(RegenDelay, FTimerDelegate::CreateLambda(
				[&]()
				{
					_fuelRegenerateDelay = false;
				}),
				5.0f, false);
		}
	}

}


void UMyCharacterParkourComponent::RegenerateGage(float deltaTime)
{
	if (!_regenerateDelay && !_bIsSprinting)
	{
		if (_stamina != 100.f && _stamina < 100.f)
		{
			_stamina += deltaTime * 10.0f;
			if (_stamina > 100.f)
			{
				_stamina = 100.f;
			}
		}
	}

	if (!_fuelRegenerateDelay && !_bIsGliding)
	{
		if (_fuel != 100.f && _fuel < 100.f)
		{
			_fuel += deltaTime * 10.0f;
			if (_fuel > 100.f)
			{
				_fuel = 100.f;
			}
		}
	}
}


void UMyCharacterParkourComponent::Glide_Implementation()
{
	_bIsGliding = true;
	Glide_Multi();
}

void UMyCharacterParkourComponent::Glide_Multi_Implementation()
{
	if (_character->IsLocallyControlled())
	{
		_springArm->SetRelativeLocation(FVector( - 400, 0, 0));
	}
	_character->_glide->SetVisibility(true);

	_movementComponent->GravityScale = 0.15f;
	_movementComponent->AirControl = 0.7f;

}

void UMyCharacterParkourComponent::UnGlide_Implementation()
{
	if (_bIsGliding)
	{
		_bIsGliding = false;
		UnGlide_Multi();
	}
}

void UMyCharacterParkourComponent::UnGlide_Multi_Implementation()
{
	if (_character->IsLocallyControlled())
	{
		_springArm->SetRelativeLocation(FVector(0,0,50));
	}
	_character->_glide->SetVisibility(false);

	_movementComponent->GravityScale = 1.0f;
	_movementComponent->AirControl = 0.5f;
}

void UMyCharacterParkourComponent::SetLedgeUpMoveDir_Implementation(float value)
{
	_ledgeUp = value;
}

void UMyCharacterParkourComponent::SetLedgeRightMoveDir_Implementation(float value)
{
	_ledgeRight = value;
}


void UMyCharacterParkourComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyCharacterParkourComponent, _bIsWallRunning);
	DOREPLIFETIME(UMyCharacterParkourComponent, _bIsledging);
	DOREPLIFETIME(UMyCharacterParkourComponent, _bIsSprinting);
	DOREPLIFETIME(UMyCharacterParkourComponent, _bIsGliding);

	DOREPLIFETIME(UMyCharacterParkourComponent, _ledgeUp);
	DOREPLIFETIME(UMyCharacterParkourComponent, _ledgeRight);

}

