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

#include "DrawDebugHelpers.h"



// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Create Instance
	_replicatorComponent = CreateDefaultSubobject<UMyCharacterReplicatorComponent>(TEXT("ReplicatorComponent"));
	_actionComponent = CreateDefaultSubobject<UMyCharacterActionComponent>(TEXT("ActionComponent"));
	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
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
	_camera->SetupAttachment(_springArm, USpringArmComponent::SocketName);
	

	//net
	_actionComponent->SetIsReplicated(true);
	_replicatorComponent->SetIsReplicated(true);
	_characterMovementComponent->SetIsReplicated(false);

	//client
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	//custom value
	_effect = 1.5f;

}

//Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnDefaultWeapon();
	}
	InitializeColor();
	InitializeInstance();
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!_actionComponent || !_replicatorComponent)
	{
		return;
	}

	FCharacterMoveInfo move = _actionComponent->CreateMove(DeltaTime);
	
	if (IsLocallyControlled())
	{
		_actionComponent->SimulateMove(move);
		_actionComponent->ObjectScan();
		_actionComponent->RespawnCheck();
	}


	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		_replicatorComponent->EnqueueAcknowledgedMove(move);
		_replicatorComponent->SendMoveToServer(move);
	}
	


}

//Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::JumpCharacter);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("showCursor", IE_Released, this, &AMyCharacter::ShowCursor);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AMyCharacter::Use);

	PlayerInputComponent->BindAction("Greet", IE_Pressed, this, &AMyCharacter::Greet);

	//PlayerInputComponent->BindAction("Option", IE_Pressed, this, &AMyCharacter::OpenOption);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

}


void AMyCharacter::InitializeColor()
{
	FTimerHandle delay;
	GetWorldTimerManager().SetTimer(delay, FTimerDelegate::CreateLambda([&]()
		{
			SetColor();
		}), 2.f, false);

	FTimerHandle delay2;
	GetWorldTimerManager().SetTimer(delay2, FTimerDelegate::CreateLambda([&]()
		{
			_playerState = GetPlayerState<AMyPlayerState>();
			SetColor_Multi(_playerState->_CharacterColor);
		}), 4.f, false);
}

void AMyCharacter::InitializeInstance()
{
	_playerState = Cast<AMyPlayerState>(GetPlayerState());
}

void AMyCharacter::SaveColor_Implementation(const FLinearColor& color)
{
	if (!IsLocallyControlled())
	{
		return;
	}
	_playerState = GetPlayerState<AMyPlayerState>();
	_playerState->_CharacterColor = color;
	_playerState->_Initialized = true;
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}

void AMyCharacter::SetColor_Implementation()
{
	UMyGameInstance* localGameInstance = GetGameInstance<UMyGameInstance>();
	SaveColor(localGameInstance->_characterInfo._characterMesh);
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), localGameInstance->_characterInfo._characterMesh);
}

void AMyCharacter::SetColor_Multi_Implementation(const FLinearColor& color)
{
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}


void AMyCharacter::SpawnDefaultWeapon_Implementation()
{
	
	if (!_equippedWeapon)
	{
		_equippedWeapon = GetWorld()->SpawnActor<AMyWeapon>(_DefaultWeapon, FVector(0, 0, -500), FRotator(), FActorSpawnParameters());
		_equippedWeapon->SetInstigator(this);
		_equippedWeapon->SetWeaponState(EWeaponState::EWS_Equip);
		_equippedWeapon->SetWeaponName(EWeaponKind::EWK_Fist);
		GetMesh()->GetSocketByName("AttackSocket")->AttachActor(_equippedWeapon, GetMesh());
	}
}



void AMyCharacter::MoveForward(float value)
{
	if (_actionComponent)
	{
		_actionComponent->SetForwardInput(value);
	}
}

void AMyCharacter::MoveRight(float value)
{
	if (_actionComponent)
	{
		_actionComponent->SetRightInput(value);
	}
}


void AMyCharacter::LookUp(float value)
{
	if (_actionComponent)
	{
		_actionComponent->SetLookUpInput(value);
	}
}

void AMyCharacter::Turn(float value)
{
	if (_actionComponent)
	{
		_actionComponent->SetRotatingInput(value);
	}
}

void AMyCharacter::JumpCharacter()
{
	if (_actionComponent)
	{
		_actionComponent->SetJumpInput(true);
	}
}

void AMyCharacter::Use()
{
	if (_actionComponent)
	{
		_actionComponent->SetUseInput(true);
	}
}

void AMyCharacter::Attack()
{
	if (_actionComponent)
	{
		_actionComponent->SetAttackInput(true);
	}
}

void AMyCharacter::Greet()
{
	if (_actionComponent)
	{
		_actionComponent->SetGreetInput(true);
	}
}

void AMyCharacter::Ride()
{
	if (_actionComponent)
	{
		_actionComponent->SetRideInput(true);
	}
}


void AMyCharacter::ShowCursor()
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (controller->bShowMouseCursor)
	{
		controller->bShowMouseCursor = false;
	}
	else
	{
		controller->bShowMouseCursor = true;
	}
	
}




