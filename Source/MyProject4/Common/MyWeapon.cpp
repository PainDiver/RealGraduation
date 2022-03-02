// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWeapon.h"
#include "Components/BoxComponent.h"
#include "MyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMyWeapon::AMyWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_skeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	_skeletalMesh->SetupAttachment(GetRootComponent());

	_combatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	_combatCollision->SetupAttachment(GetRootComponent());

	_combatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_combatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	_combatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	_combatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	_eWeaponState = EWeaponState::EWS_Pickup;
}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (_combatCollision)
	{
		_combatCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyWeapon::CombatOnOverlapBegin);
	}
}

// Called every frame
void AMyWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor!=GetInstigator())
	{
		LaunchServer(OtherActor,_eWeaponName);
	}
}

void AMyWeapon::LaunchServer_Implementation(AActor* OtherActor,const EWeaponKind& kind)
{
	switch (kind)
	{
	case EWeaponKind::EWK_Fist:
	{	
		AMyCharacter* pawn = Cast<AMyCharacter>(OtherActor);
		if (pawn)
		{
			FVector dir{ pawn->GetActorLocation() - GetActorLocation() };
			dir.Normalize();
			pawn->LaunchCharacter(dir * 700, false, true);
		}
	}
	default:
		break;
	}
}



//void AMyWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
//	//if ((_EweaponState == EWeaponState::EWS_Pickup) && OtherActor)
//	//{
//	//	AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
//
//	//	if (character)
//	//	{
//	//		character->SetOverlappingItem(this);
//	//	}
//	//}
//	//æ∆¿Ã≈€ ≈âµÊøÎ
//}
//
//
//void AMyWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
//	//if (OtherActor)
//	//{
//	//	AMyCharacter* character = Cast<AMyCharacter>(OtherActor);
//
//	//	if (character)
//	//	{
//	//		character->SetOverlappingItem(nullptr);
//	//	}
//	//}
//	//æ∆¿Ã≈€ ≈âµÊøÎ
//}


void AMyWeapon::Equip(AMyCharacter* character)
{
	//if (character)
	//{
	//	_skeletalMesh = character->GetMesh();
	//	_skeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//	_skeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//	_skeletalMesh->SetSimulatePhysics(false);

	//	const USkeletalMeshSocket* AttackSocket = _skeletalMesh->GetSocketByName("AttackSocket");
	//	if (AttackSocket)
	//	{
	//		AttackSocket->AttachActor(this, character->GetMesh());
	//		_bRotate = false;

	//		character->SetEquippedWeapon(this);
	//		character->SetOverlappingItem(nullptr);
	//	}
	//}
	// æ∆¿Ã≈€ ≈âµÊøÎ
}