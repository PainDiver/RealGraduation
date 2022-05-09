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
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	//custom value
	_effect = 1.5f;

}

//Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	auto controller = Cast<APlayerController>(GetController());
	if (controller)
	{
		DisableInput(controller);
	}
	if (IsLocallyControlled())
	{
		SpawnDefaultWeapon();
	}

	if (_glide)
	{
		_glide->SetVisibility(false);
	}

	if (IsLocallyControlled()) // use only for listen server host migration
	{
		EnableInput(controller);
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

	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetActorLocation().Z < -15000 || GetActorLocation().Z > 10000)
		{
			_replicatorComponent->Respawn();
		}
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
	if (!_actionComponent||!_characterMovementComponent)
	{
		return;
	}


	if (_parkourComponent&& _parkourComponent->IsActive() &&_parkourComponent->_bIsledging)
	{	
		if (abs(value) > 0.2)
		{
			if (value > 0)
			{
				if (_parkourComponent->_upWall)
				{
					_parkourComponent->SetLedgeUpMoveDir(value);
					ClimbWall(GetCapsuleComponent()->GetRelativeLocation() + _parkourComponent->_ledgeUpMoveDir * 50.f);
					auto newRot = _parkourComponent->_newRot;
					newRot.Pitch -= 360.f;
					GetMesh()->SetRelativeRotation(newRot);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Up Wall"));
					_parkourComponent->LedgeJump();
				}
			}
			else
			{
				if (_parkourComponent->_downWall)
				{
					_parkourComponent->SetLedgeUpMoveDir(value);
					ClimbWall(GetCapsuleComponent()->GetRelativeLocation() + _parkourComponent->_ledgeUpMoveDir * -50.f);
					auto newRot = _parkourComponent->_newRot;
					newRot.Pitch -= 360.f;
					GetMesh()->SetRelativeRotation(newRot);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Down Wall"));
					_parkourComponent->LedgeJump();
				}
			}
		}
		else
		{
			_parkourComponent->SetLedgeUpMoveDir(value);
		}
		
		return;
	}
	
	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(direction, value);


	if (_actionComponent && _characterMovementComponent && _parkourComponent && _parkourComponent->IsActive())
	{
		if(_parkourComponent->_bIsWallRunning&& !_characterMovementComponent->IsFalling())
		_actionComponent->StartWalkCamShake(abs(value));
	}

	if (_parkourComponent && _parkourComponent->IsActive() &&_parkourComponent->_bIsSprinting && value < 0)
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

	if (_parkourComponent&& _parkourComponent->IsActive() &&_parkourComponent->_bIsledging)
	{
		if (abs(value) > 0.2)
		{
			if (value > 0)
			{
				if (_parkourComponent->_rightWall)
				{
					_parkourComponent->SetLedgeRightMoveDir(value);
					ClimbWall(GetCapsuleComponent()->GetRelativeLocation()+ _parkourComponent->_ledgeRightMoveDir * 40.f);				
					auto newRot = _parkourComponent->_newRot;
					newRot.Pitch -= 360.f;
					GetMesh()->SetRelativeRotation(newRot);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Right Wall"));
					_parkourComponent->LedgeJump();
				}
			}
			else
			{
				if (_parkourComponent->_leftWall)
				{
					_parkourComponent->SetLedgeRightMoveDir(value);
					ClimbWall(GetCapsuleComponent()->GetRelativeLocation()+ _parkourComponent->_ledgeRightMoveDir * -40.f);
					auto newRot = _parkourComponent->_newRot;
					newRot.Pitch -= 360.f;
					GetMesh()->SetRelativeRotation(newRot);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Left Wall"));
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

	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(direction, value);

	if (_actionComponent&&_characterMovementComponent&& _parkourComponent&& _parkourComponent->IsActive())
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

	if (!_parkourComponent && _parkourComponent->IsActive() || _parkourComponent->_bIsledging)
	{
		return;
	}

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

	if (_parkourComponent && _parkourComponent->_bIsGliding)
	{
		_parkourComponent->UnGlide();
	}

	if (_parkourComponent && _parkourComponent->_bIsWallRunning)
	{
		_parkourComponent->StopWallRunning();
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

	if (_characterMovementComponent->MovementMode==EMovementMode::MOVE_Swimming)
	{
		_actionComponent->SwimJump();
		return;
	}

	ACharacter::Jump();
	
	FVector lastWorldVector = _characterMovementComponent->GetLastInputVector();
	FTransform transform = GetActorTransform();
	FVector lastLocalVector = UKismetMathLibrary::InverseTransformDirection(transform, lastWorldVector);

	// delay, ledging, falling, moving, back, last input, wall running �˻� �ʿ�

	if (_parkourComponent->IsActive()&&!_parkourComponent->_bIsGliding && _characterMovementComponent->IsFalling() && _characterMovementComponent->Velocity.Size() >= 500 && lastLocalVector.X > 0 && !_parkourDelay)
	{
		_parkourComponent->WallRunLeft();
		_parkourComponent->WallRunRight();
		return;
	}

	if (HasAuthority())
	{
		if (_parkourComponent&& _parkourComponent->IsActive() && _characterMovementComponent->IsFalling())
		{
			if (_parkourComponent->_bIsledging && !_parkourDelay)
			{
				_parkourComponent->LedgeJump();
				DelayParkour();
			}
			else if (!_parkourComponent->_bIsWallRunning && _parkourComponent->_fuel)
			{
				_parkourComponent->Glide();
			}
		}
	}
	//jump released ��, wallrunning ���� �˻� �� delay, stop wall running, launch, camera shake �����ʿ�
}


void AMyCharacter::JumpRelease()
{
	ACharacter::StopJumping();

	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsWallRunning && !_parkourDelay)
	{
		_parkourComponent->StopWallRunning();
		_parkourComponent->WallRunningJump();
		DelayParkour();
		//camerashake
	}
	if (_parkourComponent && _parkourComponent->IsActive() && _parkourComponent->_bIsGliding)
	{
		_parkourComponent->UnGlide();
	}



}

void AMyCharacter::Sprint()
{

	FVector lastWorldVector = _characterMovementComponent->GetLastInputVector();
	FTransform transform = GetActorTransform();
	FVector lastLocalVector = UKismetMathLibrary::InverseTransformDirection(transform, lastWorldVector);


	if (_parkourComponent&& _parkourComponent->IsActive() && lastLocalVector.X > 0 && lastLocalVector.Y != 1.0f && _parkourComponent->_stamina)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Fire Sprint"));
		_parkourComponent->Sprint();
	}
}

void AMyCharacter::SprintRelease()
{
	if (_parkourComponent&& _parkourComponent->IsActive() && _parkourComponent->_bIsSprinting)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Stop Sprint"));
		_parkourComponent->StopSprint();
	}
}



void AMyCharacter::Ledge()
{
	if (_parkourComponent&& _parkourComponent->IsActive() && !_parkourDelay && _characterMovementComponent->IsFalling())
	{
		_parkourComponent->Ledge();
		DelayParkour();
	}
}

void AMyCharacter::UnLedge()
{
	if (_parkourComponent&& _parkourComponent->IsActive() && _parkourComponent->_bIsledging)
	{
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
		0.3f, false);
}


void AMyCharacter::ClimbWall_Implementation(const FVector& vec)
{
	auto newRot = GetCapsuleComponent()->GetRelativeRotation();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), vec, newRot, true, true, 0.05f, true, EMoveComponentAction::Move, LatentInfo);

}



void AMyCharacter::FlyUp_AllMighty_Server_Implementation(float value)
{
	FlyUp_AllMighty_Multi(value);
}


void AMyCharacter::FlyUp_AllMighty_Multi_Implementation(float value)
{
	FVector currentLoc = GetActorLocation();
	FVector newLoc = currentLoc + 25 * GetActorUpVector() * value;
	SetActorLocation(newLoc);
}

void AMyCharacter::MoveForward_AllMighty_Server_Implementation(float value)
{
	MoveForward_AllMighty_Multi(value);
}

void AMyCharacter::MoveForward_AllMighty_Multi_Implementation(float value)
{
	FVector currentLoc = GetActorLocation();
	FRotator currentRot = GetControlRotation();
	FRotationMatrix rotmat = FRotationMatrix(currentRot);
	FVector dir = rotmat.GetUnitAxis(EAxis::X);
	FVector newLoc = currentLoc + 25 * value * dir;
	SetActorLocation(newLoc);
}


void AMyCharacter::MoveRight_AllMighty_Server_Implementation(float value)
{
	MoveRight_AllMighty_Multi(value);
}

void AMyCharacter::MoveRight_AllMighty_Multi_Implementation(float value)
{
	FVector currentLoc = GetActorLocation();
	FVector newLoc = currentLoc + 25 * GetActorRightVector() * value;
	SetActorLocation(newLoc);
}


void AMyCharacter::AllMightyModeBinding()
{
	InputComponent->AxisBindings.Empty();

	InputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	InputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
	InputComponent->BindAxis("FlyUp", this, &AMyCharacter::FlyUp_AllMighty_Server);
	InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward_AllMighty_Server);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight_AllMighty_Server);
}


void AMyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyCharacter, _parkourDelay)
}

