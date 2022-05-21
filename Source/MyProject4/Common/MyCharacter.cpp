// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../PlayMap/MyWorldTimer.h"
#include "../PlayMap/MyPickups.h"
#include "MyGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "../PlayMap/MyPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "../Common/MyWeapon.h"
#include "GameFrameWork/GameStateBase.h"
#include "MyCharacterActionComponent.h"
#include "MyCharacterReplicatorComponent.h"
#include "MySaveGame.h"
#include "UMG.h"
#include "MyCharacterParkourComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"



// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Instance
	_parkourComponent = CreateDefaultSubobject<UMyCharacterParkourComponent>(TEXT("ParkourComponent"));
	_replicatorComponent = CreateDefaultSubobject<UMyCharacterReplicatorComponent>(TEXT("ReplicatorComponent"));
	_actionComponent = CreateDefaultSubobject<UMyCharacterActionComponent>(TEXT("ActionComponent"));
	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	_glide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glide"));


	_characterMovementComponent = GetCharacterMovement();

	//collider Set
	GetCapsuleComponent()->SetCapsuleSize(30.f, 90.f);

	//control and Camera Setting
	_springArm->TargetArmLength = 10.f;
	_springArm->bUsePawnControlRotation = true;
	_camera->bUsePawnControlRotation = false;

	_characterMovementComponent->bUseControllerDesiredRotation = false;
	_characterMovementComponent->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	_characterMovementComponent->RotationRate = FRotator(0.f, 540.f, 0.f);
	_characterMovementComponent->JumpZVelocity = 700.f;
	_characterMovementComponent->AirControl = 0.5f;
	_characterMovementComponent->MaxWalkSpeed = 500.f;

	//Attachment
	_springArm->SetupAttachment(GetRootComponent());
	_glide->SetupAttachment(GetMesh());
	_camera->SetupAttachment(_springArm, USpringArmComponent::SocketName);


	//net
	_characterMovementComponent->SetIsReplicated(true);
	_actionComponent->SetIsReplicated(true);
	_replicatorComponent->SetIsReplicated(true);
	_parkourComponent->SetIsReplicated(true);

	//client
	//AutoPossessPlayer = EAutoReceiveInput::Player0;

	//custom value
	_effect = 1.5f;

}

//Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocallyControlled())
	{
		SpawnDefaultWeapon();
		UMyGameInstance* localInstance = Cast<UMyGameInstance>(GetGameInstance());
		if (!localInstance)
		{
			return;
		}
		SetColor(localInstance->_characterInfo._characterColor);
	}

	if (_glide)
	{
		_glide->SetVisibility(false);
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server Confirmed Character is Generated"));
	}
	else if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Self Confirmed Character is Generated"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Other Confirmed Character is Generated"));
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsPawnControlled())
	{
		return;
	}
	if (!_actionComponent || !_replicatorComponent || !_parkourComponent)
	{
		return;
	}

	if (GetActorLocation().Z < -15000)
	{
		_replicatorComponent->Respawn();
	}
	

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		_parkourComponent->CheckGage(DeltaTime);
		_parkourComponent->RegenerateGage(DeltaTime);
	}


}

//Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyCharacter::JumpRelease);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyCharacter::SprintRelease);



	PlayerInputComponent->BindAction("showCursor", IE_Released, this, &AMyCharacter::ShowCursor);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::Interact);

	PlayerInputComponent->BindAction("Greet", IE_Pressed, this, &AMyCharacter::Greet);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);

	PlayerInputComponent->BindAction("Ledge", IE_Pressed, this, &AMyCharacter::Ledge);
	PlayerInputComponent->BindAction("Ledge", IE_Released, this, &AMyCharacter::UnLedge);



	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	//PlayerInputComponent->BindAction("Option", IE_Pressed, this, &AMyCharacter::OpenOption);

}

void AMyCharacter::SetController_Client_Implementation(bool signal)
{
	if (signal)
	{
		auto controller = Cast<APlayerController>(GetController());
		if (controller && IsLocallyControlled())
		{
			EnableInput(controller);
			controller->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		auto controller = Cast<APlayerController>(GetController());
		if (controller && IsLocallyControlled())
		{
			DisableInput(controller);
		}
	}
}


void AMyCharacter::SpawnDefaultWeapon_Implementation()
{
	FActorSpawnParameters param;
	param.Owner = this;
	param.Instigator = this;

	if (_defaultWeaponClass)
	{
		_equippedWeapon = GetWorld()->SpawnActor<AMyWeapon>(_defaultWeaponClass, param);
	}
	if (_equippedWeapon)
	{
		_equippedWeapon->SetWeaponState(EWeaponState::EWS_Equip);
		_equippedWeapon->SetWeaponName(EWeaponKind::EWK_Fist);
		GetMesh()->GetSocketByName("AttackSocket")->AttachActor(_equippedWeapon, GetMesh());
	}
}

void AMyCharacter::InitializeInstance()
{
	_playerState = Cast<AMyPlayerState>(GetPlayerState());
}


void AMyCharacter::SetColor_Implementation(const FLinearColor& color)
{
	SetColor_Multi(color);
}

void AMyCharacter::SetColor_Client_Implementation(const FLinearColor& color)
{
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}


void AMyCharacter::SetColor_Multi_Implementation(const FLinearColor& color)
{
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}


void AMyCharacter::MoveForward(float value)
{
	if (!_actionComponent || !_characterMovementComponent)
	{
		return;
	}

	if (_parkourComponent && _parkourComponent->IsActive())
	{
		if (_parkourComponent->_bIsledging)
		{
			if (abs(value) > 0.2)
			{
				if (value > 0)
				{
					if (_parkourComponent->_upWall)
					{
						_parkourComponent->SetLedgeUpMoveDir(value);
						SetActorLocation(GetActorLocation() + _parkourComponent->_ledgeUpMoveDir * 15.f); 
						auto newRot = _parkourComponent->_upRot;
						GetCapsuleComponent()->SetRelativeRotation(newRot);
						CorrectLocation_Server(GetActorLocation());

					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("No Up Wall"));
						CorrectLocation_Server(GetActorLocation());
						_parkourComponent->LedgeJump();
					}
				}
				else
				{
					if (_parkourComponent->_feetWall)
					{
						UE_LOG(LogTemp, Warning, TEXT("No Down Wall"));
						CorrectLocation_Server(GetActorLocation());
						_parkourComponent->LedgeJump();
					}
					else if (_parkourComponent->_downWall)
					{
						_parkourComponent->SetLedgeUpMoveDir(value);
						SetActorLocation(GetActorLocation() + _parkourComponent->_ledgeDownMoveDir * 15.f);
						//ClimbWall(GetCapsuleComponent()->GetRelativeLocation() + _parkourComponent->_ledgeDownMoveDir * 30.f);
						auto newRot = _parkourComponent->_upRot;
						GetCapsuleComponent()->SetRelativeRotation(newRot);
						CorrectLocation_Server(GetActorLocation());
					}
					
				}
			}
			else
			{
				_parkourComponent->SetLedgeUpMoveDir(value);
			}

			return;
		}
	}

	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(direction, value);


	if (_actionComponent && _characterMovementComponent && _parkourComponent && _parkourComponent->IsActive())
	{
		if (_parkourComponent->_bIsWallRunning || !_characterMovementComponent->IsFalling())
		{
			_actionComponent->StartWalkCamShake(abs(value));
		}
	}

	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsSprinting && value < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire SprintStop"));
		_parkourComponent->StopSprint();
	}

	//if (_actionComponent)
	//{
	//	_actionComponent->SetForwardInput(value);
	//}
}

void AMyCharacter::MoveRight(float value)
{
	//_actionComponent->AddMoveRight(0, value);

	if (_parkourComponent && _parkourComponent->IsActive())
	{
		if (_parkourComponent->_bIsledging)
		{
			if (abs(value) > 0.2)
			{
				if (value > 0)
				{
					if (_parkourComponent->_rightWall)
					{
						_parkourComponent->SetLedgeRightMoveDir(value);
						SetActorLocation(GetActorLocation()+ _parkourComponent->_ledgeRightMoveDir * 10.f);
						CorrectLocation_Server(GetActorLocation());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("No Right Wall"));
						CorrectLocation_Server(GetActorLocation());
						_parkourComponent->LedgeJump();
					}
				}
				else
				{
					if (_parkourComponent->_leftWall)
					{
						_parkourComponent->SetLedgeRightMoveDir(value);
						SetActorLocation(GetActorLocation() + _parkourComponent->_ledgeLeftMoveDir * 10.f);
						CorrectLocation_Server(GetActorLocation());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("No Left Wall"));
						CorrectLocation_Server(GetActorLocation());
						_parkourComponent->LedgeJump();
					}
				}
			}
			else
			{
				_parkourComponent->SetLedgeRightMoveDir(value);
			}
			return;
		}
	}
	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(direction, value);

	if (_actionComponent && _characterMovementComponent && _parkourComponent && _parkourComponent->IsActive())
	{
		if (!_characterMovementComponent->IsFalling() || _parkourComponent->_bIsWallRunning)
		{
			_actionComponent->StartWalkCamShake(abs(value));
		}
	}

	//if (_actionComponent)
	//{
	//	_actionComponent->SetRightInput(value);
	//}
}


void AMyCharacter::LookUp(float value)
{
	//_actionComponent->AddLookUp(0, value);

	FRotator baseAimRotator = GetBaseAimRotation();

	if (value >= 0)
	{
		if (baseAimRotator.Pitch > -65.f)
		{
			AddControllerPitchInput(value);
		}
	}
	else
	{
		if (baseAimRotator.Pitch < 45.f)
		{
			AddControllerPitchInput(value);
		}
	}
	//if (_actionComponent)
	//{
	//	_actionComponent->SetLookUpInput(value);
	//}
}

void AMyCharacter::Turn(float value)
{
	

	//_actionComponent->AddRotation(0, value);
	AddControllerYawInput(value);

	//if (_actionComponent)
	//{
	//	_actionComponent->SetRotatingInput(value);
	//}
}

void AMyCharacter::Interact()
{
	if (_actionComponent)
	{
		_actionComponent->Interact(true);
	}
	//if (_actionComponent)
	//{
	//	_actionComponent->SetInteractInput(true);
	//}
}

void AMyCharacter::Attack()
{
	if (_actionComponent)
	{
		_actionComponent->Attack(true);
	}
	//if (_actionComponent)
	//{
	//	_actionComponent->SetAttackInput(true);
	//}
}

void AMyCharacter::Greet()
{
	if (_actionComponent)
	{
		_actionComponent->Greet(true);
	}
	//if (_actionComponent)
	//{
	//	_actionComponent->SetGreetInput(true);
	//}
}





void AMyCharacter::ShowCursor()
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (controller->bShowMouseCursor)
	{
		controller->SetInputMode(FInputModeGameOnly());
		controller->bShowMouseCursor = false;
	}
	else
	{
		controller->SetInputMode(FInputModeGameAndUI());
		controller->bShowMouseCursor = true;
	}

}

void AMyCharacter::Landed(const FHitResult& Hit)
{
	ACharacter::Landed(Hit);



	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsGliding)
	{
		_parkourComponent->UnGlide();
	}

	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsWallRunning)
	{
		_parkourComponent->StopWallRunning(GetActorLocation());
	}

}

void AMyCharacter::OnJumped()
{
	ACharacter::OnJumped();

	if (_actionComponent)
	{
		_actionComponent->StartJumpCamShake();
	}
}

void AMyCharacter::Jump()
{
	if (!_characterMovementComponent || !_parkourComponent || !_actionComponent)
	{
		return;
	}

	if (_characterMovementComponent->MovementMode == EMovementMode::MOVE_Swimming)
	{
		_actionComponent->SwimJump();
		return;
	}

	ACharacter::Jump();

	FVector lastWorldVector = _characterMovementComponent->GetLastInputVector();
	FTransform transform = GetActorTransform();
	FVector lastLocalVector = UKismetMathLibrary::InverseTransformDirection(transform, lastWorldVector);

	// delay, ledging, falling, moving, back, last input, wall running 검사 필요

	if (_parkourComponent && _parkourComponent->IsActive())
	{
		if (!_parkourComponent->_bIsGliding && _characterMovementComponent->IsFalling() && _characterMovementComponent->Velocity.Size() >= 500 && lastLocalVector.X > 0 && !_parkourDelay)
		{
			_parkourComponent->WallRunLeft();
			_parkourComponent->WallRunRight();
			return;
		}

		if (_characterMovementComponent->IsFalling())
		{
			if (_parkourComponent->_bIsledging && !_parkourDelay)
			{
				CorrectLocation_Server(GetActorLocation());
				_parkourComponent->LedgeJump();
				DelayParkour();
			}
			else if (!_parkourComponent->_bIsWallRunning && _parkourComponent->_fuel)
			{
				if (!_parkourComponent->_bIsGliding && !_parkourComponent->_bUsingInteractable)
				{
					_parkourComponent->Glide();

					DelayParkour();
				}
			}

		}
	}
	//jump released 시, wallrunning 여부 검사 후 delay, stop wall running, launch, camera shake 실행필요
}


void AMyCharacter::JumpRelease()
{
	ACharacter::StopJumping();

	if (_parkourComponent && _parkourComponent->IsActive())
	{
		if (_parkourComponent->_bIsWallRunning && !_parkourDelay)
		{
			_parkourComponent->StopWallRunning(GetActorLocation());
			_parkourComponent->WallRunningJump(GetActorLocation());
			DelayParkour();
			//camerashake
		}
		if (_parkourComponent->_bIsGliding)
		{
			_parkourComponent->UnGlide();
		}
	}


}

void AMyCharacter::Sprint()
{

	FVector lastWorldVector = _characterMovementComponent->GetLastInputVector();
	FTransform transform = GetActorTransform();
	FVector lastLocalVector = UKismetMathLibrary::InverseTransformDirection(transform, lastWorldVector);


	if (_parkourComponent && _parkourComponent->IsActive() && lastLocalVector.X > 0 && lastLocalVector.Y != 1.0f && _parkourComponent->_stamina)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Fire Sprint"));
		_parkourComponent->Sprint();
	}
}

void AMyCharacter::SprintRelease()
{
	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsSprinting)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Stop Sprint"));
		_parkourComponent->StopSprint();
	}
}



void AMyCharacter::Ledge()
{
	if (_parkourComponent && _parkourComponent->IsActive() && !_parkourDelay && _characterMovementComponent->IsFalling())
	{
		_parkourComponent->Ledge();
		DelayParkour();
	}
}

void AMyCharacter::UnLedge()
{
	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsledging)
	{
		CorrectLocation_Server(GetActorLocation());
		_parkourComponent->Unledge();
		DelayParkour();
	}
}

void AMyCharacter::DelayParkour_Implementation()
{
	_parkourDelay = true;
	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer, FTimerDelegate::CreateLambda(
		[&]()
		{
			_parkourDelay = false;
		}),
		0.4f, false);
}


void AMyCharacter::ClimbWall_Implementation(const FVector& vec)
{
	ClimbWall_Multi(vec);
}

void AMyCharacter::ClimbWall_Multi_Implementation(const FVector& vec)
{
	auto newRot = GetCapsuleComponent()->GetRelativeRotation();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), vec, newRot, true, true, 0.05f, true, EMoveComponentAction::Move, LatentInfo);

}



void AMyCharacter::FlyUp_AllMighty(float value)
{
	if (value == 0)
	{
		return;
	}
	//FlyUp_AllMighty_Multi(value);
	AddMovementInput(GetActorUpVector(), value);

}

void AMyCharacter::MoveForward_AllMighty(float value)
{
	if (value == 0)
	{
		return;
	}
	//FVector currentLoc = GetActorLocation();
	FRotator viewDir = GetControlRotation();
	FVector dir = viewDir.Vector();
	//FVector newLoc = currentLoc + 150 * value * dir;
	AddMovementInput(dir, value);

	//CorrectLocation_Server(newLoc);
}

void AMyCharacter::MoveRight_AllMighty(float value)
{
	if (value == 0)
	{
		return;
	}
	//FVector currentLoc = GetActorLocation();
	//FVector newLoc = currentLoc + 150 * GetActorRightVector() * value;
	//SetActorLocation(newLoc);

	AddMovementInput(GetActorRightVector(), value);
}

void AMyCharacter::CorrectLocation_Server_Implementation(const FVector& loc)
{
	SetActorLocation(loc);
}

void AMyCharacter::CorrectLocation_Multi_Implementation(const FVector& loc)
{
	SetActorLocation(loc);
}

void AMyCharacter::SetSpeed_Server_Implementation(const float& effect)
{
	SetSpeed_Multi(effect);
}

void AMyCharacter::SetSpeed_Multi_Implementation(const float& effect)
{
	_characterMovementComponent->MaxWalkSpeed += effect;
}



void AMyCharacter::AllMightyModeBinding()
{
	InputComponent->AxisBindings.Empty();

	InputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	InputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
	InputComponent->BindAxis("FlyUp", this, &AMyCharacter::FlyUp_AllMighty);
	InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward_AllMighty);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight_AllMighty);
}


void AMyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyCharacter, _parkourDelay)
}

