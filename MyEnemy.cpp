// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnemy.h"
#include "MyCharacter.h"
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
	_agroSphere->InitSphereRadius(600.f);

	_staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	_staticMesh->SetupAttachment(GetRootComponent());

	_numOfPatrolPoints = 10;

	_ems = EEnemyMovementStatus::EMS_Idle;

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

// Called when the game starts or when spawned
void AMyEnemy::BeginPlay()
{
	Super::BeginPlay();

	_AIController = Cast<AAIController>(GetController());

	UE_LOG(LogTemp, Warning, TEXT("Enemy Showed up"));

	for (int i=0;i<_numOfPatrolPoints;i++)
	{
		_patrolPoints.Add(FVector( (rand()%2==1?-1:1) *rand()%500, (rand() % 2 == 1 ? -1 : 1) * rand()%500, GetActorLocation().Z));
	}

	_agroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	_agroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	_agroSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOnOverlapBegin);
	_agroSphere->OnComponentEndOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOnOverlapEnd);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	switch (_ems)
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
	if (OtherActor)
	{
		AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
		if (character)
		{
			SetState(EEnemyMovementStatus::EMS_Chasing);
			UE_LOG(LogTemp, Warning, TEXT("OverlapBeginByCharacter!"));
			_AIController->StopMovement();
			_target = character;
			MoveToTarget(character);
		}

	}
}
void AMyEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
		if (character)
		{
				_AIController->StopMovement();
				_target = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("PatrolStart!"));
				Patrol();
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

void AMyEnemy::SetState(EEnemyMovementStatus state)
{
	_ems = state;
}

