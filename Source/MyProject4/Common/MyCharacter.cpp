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
	auto controller = Cast<APlayerController>(GetController());
	if (controller)
	{
		DisableInput(controller);
	}
	if (IsLocallyControlled())
	{
		SpawnDefaultWeapon();
	}
	
	if (IsLocallyControlled()) // use only for listen server host migration
	{
		//UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);
		_saveGame = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0));
		if (_saveGame)
		{
			if (HasAuthority())
			{
				_saveGame->ServerLoad(GetWorld());
			}
			else
			{
				_saveGame->ClientLoad(GetWorld());
			}
		}
		UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);
	}
	EnableInput(controller);

	GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue,"initialized character");
	UE_LOG(LogTemp, Warning, TEXT("Character Okay"));
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsPawnControlled())
	{
		return;
	}
	if (!_actionComponent || !_replicatorComponent)
	{
		return;
	}

	//FCharacterMoveInfo move = _actionComponent->CreateMove(DeltaTime);
	//if (GetLocalRole() == ROLE_AutonomousProxy)
	//{
	//	_replicatorComponent->EnqueueAcknowledgedMove(move);
	//	_replicatorComponent->SendMoveToServer(move);
	//}
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetActorLocation().Z < -2000 || GetActorLocation().Z > 8000)
		{
			_replicatorComponent->Respawn();
		}
	}
	//if (IsLocallyControlled())
	//{
	//	_actionComponent->SimulateMove(move);
	//}
	
}

//Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("showCursor", IE_Released, this, &AMyCharacter::ShowCursor);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::Interact);

	PlayerInputComponent->BindAction("Greet", IE_Pressed, this, &AMyCharacter::Greet);

	//PlayerInputComponent->BindAction("Option", IE_Pressed, this, &AMyCharacter::OpenOption);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

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
	_actionComponent->AddMoveForward(0,value);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetForwardInput(value);
	//}
}

void AMyCharacter::MoveRight(float value)
{
	_actionComponent->AddMoveRight(0, value);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetRightInput(value);
	//}
}


void AMyCharacter::LookUp(float value)
{
	_actionComponent->AddLookUp(0, value);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetLookUpInput(value);
	//}
}

void AMyCharacter::Turn(float value)
{
	_actionComponent->AddRotation(0, value);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetRotatingInput(value);
	//}
}

void AMyCharacter::Interact()
{
	_actionComponent->Interact(true);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetInteractInput(true);
	//}
}

void AMyCharacter::Attack()
{
	_actionComponent->Attack(true);
	//if (_actionComponent)
	//{
	//	_actionComponent->SetAttackInput(true);
	//}
}

void AMyCharacter::Greet()
{
	_actionComponent->Greet(true);
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
