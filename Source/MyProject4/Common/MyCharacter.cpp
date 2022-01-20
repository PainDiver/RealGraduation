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




// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_springArm->SetupAttachment(GetRootComponent());
	_springArm->TargetArmLength = 10.f;
	_springArm->bUsePawnControlRotation = true;

	_spectatorArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpectatorArm"));
	_spectatorArm->SetupAttachment(GetRootComponent());
	_spectatorArm->TargetArmLength = 40.f;
	_spectatorArm->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(30.f, 90.f);


	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	_camera->SetupAttachment(_springArm, USpringArmComponent::SocketName);
	_camera->bUsePawnControlRotation = false;

	_baseTurnRate = 65.f;
	_baseLookUpRate = 65.f;

	_respawn = FVector(0.f, 0.f, 5.f);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	_characterMovementComponent = GetCharacterMovement();

	_characterMovementComponent->bUseControllerDesiredRotation = false;
	_characterMovementComponent->bOrientRotationToMovement = true;

	_characterMovementComponent->RotationRate = FRotator(0.f, 540.f, 0.f);
	_characterMovementComponent->JumpZVelocity = 700.f;
	_characterMovementComponent->AirControl = 0.5f;
	_characterMovementComponent->MaxWalkSpeed = 500.f;

	_characterMovementComponent->SetIsReplicated(true);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	_effect = 1.5f;




}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnDefaultWeapon();
	}
	
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

void AMyCharacter::SetColor_Implementation()
{
	UMyGameInstance* localGameInstance = GetGameInstance<UMyGameInstance>();
	SaveColor(localGameInstance->_characterInfo._characterMesh);
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), localGameInstance->_characterInfo._characterMesh);
}

void AMyCharacter::SaveColor_Implementation(FLinearColor color)
{
	_playerState = GetPlayerState<AMyPlayerState>();
	_playerState->_CharacterColor = color;
	_playerState->_Initialized = true;

	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}

void AMyCharacter::SetColor_Multi_Implementation(FLinearColor color)
{
	GetMesh()->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), color);
}


void AMyCharacter::SpawnDefaultWeapon_Implementation()
{
	if (!_equippedWeapon)
	{
		_equippedWeapon = GetWorld()->SpawnActor<AMyWeapon>(_DefaultWeapon, FVector(0, 0, -500), FRotator(), FActorSpawnParameters());
		_equippedWeapon->SetInstigator(this);
		_equippedWeapon->_EweaponState = EWeaponState::EWS_Equip;
		GetMesh()->GetSocketByName("AttackSocket")->AttachActor(_equippedWeapon, GetMesh());
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetActorLocation().Z < -2000)
	{
		SetActorLocation(_respawn);
	}

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("showCursor", IE_Released, this, &AMyCharacter::ShowCursor);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AMyCharacter::UseItem);

	PlayerInputComponent->BindAction("Greet", IE_Pressed, this, &AMyCharacter::Greet);


	//PlayerInputComponent->BindAction("Option", IE_Pressed, this, &AMyCharacter::OpenOption);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyCharacter::LookUpAtRate);



}


void AMyCharacter::MoveForward(float value)
{

	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(direction, value);

}

void AMyCharacter::MoveRight(float value)
{
	FRotator rotation = GetControlRotation();
	FRotator yawRotation(0.f, rotation.Yaw, 0.f);
	FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(direction, value);

}

void AMyCharacter::LookUp(float value)
{
	FRotator baseAimRotator = GetBaseAimRotation();

	if (value >= 0)
	{
		if (baseAimRotator.Pitch > -55.f)
		{
			AddControllerPitchInput(value);
		}
	}
	else
	{
		if (baseAimRotator.Pitch < 35.f)
		{
			AddControllerPitchInput(value);
		}
	}

}


void AMyCharacter::TurnAtRate(float rate)
{
	AddControllerYawInput(rate * _baseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::LookUpAtRate(float rate)
{
	AddControllerPitchInput(rate * _baseLookUpRate * GetWorld()->GetDeltaSeconds());
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

void AMyCharacter::Respawn_Implementation()
{
	SetActorLocation(_respawn);
}


void AMyCharacter::UseItem_Implementation()
{
	_playerState = Cast<AMyPlayerState>(GetPlayerState());
	if (_playerState->_Inventory.Num() == 0)
		return;

	AMyPickups* item = _playerState->_Inventory.Pop();

	if (item)
	{
		item->Activate();
	}
}


void AMyCharacter::Attack_Implementation()
{
	if (_characterMovementComponent->IsFalling() || _IsAttacking)
	{
		return;
	}
	_characterMovementComponent->MaxWalkSpeed = 200;
	_IsAttacking = true;

}

void AMyCharacter::Greet_Implementation()
{
	_characterMovementComponent = GetCharacterMovement();
	if (_characterMovementComponent->IsFalling()||_IsAttacking || _IsGreeting)
	{
		return;
	}
	_characterMovementComponent->MaxWalkSpeed = 200;

	UE_LOG(LogTemp, Warning, TEXT("greeting"));
	_IsGreeting = true;
}


void AMyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyCharacter, _IsAttacking);
	DOREPLIFETIME(AMyCharacter, _IsGreeting);
}

//void AMyCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(AMyCharacter, _itemOn);
//}