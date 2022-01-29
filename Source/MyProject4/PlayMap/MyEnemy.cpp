// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnemy.h"
#include "../Common/MyCharacter.h"
#include "AIController.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AMyEnemy::AMyEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	_agroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	_agroSphere->SetupAttachment(GetRootComponent());
	_agroSphere->InitSphereRadius(1000.f);

	_attackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
	_attackSphere->SetupAttachment(GetRootComponent());
	_attackSphere->InitSphereRadius(120.f);

	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_staticMesh->SetupAttachment(GetRootComponent());

	_numOfPatrolPoints = 25;

	_eMovementStatus = EEnemyMovementStatus::EMS_Idle;

	GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

// Called when the game starts or when spawned
void AMyEnemy::BeginPlay()
{
	Super::BeginPlay();

	_AIController = Cast<AAIController>(GetController());

	UE_LOG(LogTemp, Warning, TEXT("Enemy Showed up"));

	_agroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	_agroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	_agroSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOnOverlapBegin);
	_agroSphere->OnComponentEndOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOnOverlapEnd);

	_attackSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::AttackSphereOnOverlapBegin);


	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	FVector loc = GetActorLocation();
	for (int i =0; i < _numOfPatrolPoints; i++)
	{
		_patrolPoints.Add(FMath::RandPointInBox(_patrolBox));
	}


}

// Called every frame
void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	switch (_eMovementStatus)
	{
		case EEnemyMovementStatus::EMS_Idle:
		{
			Patrol();
			break;
		}

		case EEnemyMovementStatus::EMS_Chasing:
		{
			MoveToTarget(_target);
			break;
		}
		default:
			break;
	}


	if ((_targetLocation - GetActorLocation()).Size() < 100.f)
	{
		SetState(EEnemyMovementStatus::EMS_Idle);
	}

}

// Called to bind functionality to input
void AMyEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}




void AMyEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor &&_AIController)
	{
		AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
		if (character)
		{
			SetState(EEnemyMovementStatus::EMS_Chasing);
			_AIController->StopMovement();
			_target = character;
			MoveToTarget(_target);
		}

	}
}

void AMyEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor&& _AIController)
	{
		AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
		if (character)
		{
				_AIController->StopMovement();
				_target = nullptr;
				Patrol();
		}
	}
}

void AMyEnemy::AttackSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
		if (character)
		{
			FVector force = (OtherActor->GetActorLocation() - GetActorLocation());
			force.Normalize();
			character->GetCharacterMovement()->AddForce(force * 50000000);
		}
	}
}


void AMyEnemy::MoveToTarget(AMyCharacter* target)
{
	if (_AIController)
	{
		FAIMoveRequest moveRequest;
		moveRequest.SetGoalActor(target);
		moveRequest.SetAcceptanceRadius(5.f);
		_targetLocation = target->GetActorLocation();
		FNavPathSharedPtr navPath;
		_AIController->MoveTo(moveRequest, &navPath);
	}
}

void AMyEnemy::Patrol()
{
	if (_AIController)
	{
		SetState(EEnemyMovementStatus::EMS_Patrol);
		FAIMoveRequest moveRequest;
		int randIndex = rand() % _numOfPatrolPoints;
		moveRequest.SetGoalLocation(_patrolPoints[randIndex]);
		_targetLocation = _patrolPoints[randIndex];
		moveRequest.SetAcceptanceRadius(5.f);
		FNavPathSharedPtr navPath;
		_AIController->MoveTo(moveRequest, &navPath);
	}
}


