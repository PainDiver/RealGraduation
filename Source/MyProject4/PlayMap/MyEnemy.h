// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyEnemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementStatus :uint8
{
	EMS_Idle UMETA(DisplayName = "Idle"),
	EMS_Patrol UMETA(DisplayName = "Patrol"),
	EMS_Chasing UMETA(DisplayName = "Chasing"),
	
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class MYPROJECT4_API AMyEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void AttackSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	void SetState(EEnemyMovementStatus state);

	void MoveToTarget(class AMyCharacter* target);

	void Patrol();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agro")
	class USphereComponent* _agroSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
		class USphereComponent* _attackSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	class AMyCharacter* _target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
		class UStaticMeshComponent* _staticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		int _numOfPatrolPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		EEnemyMovementStatus _ems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
		FBox _patrolBox;

	class AAIController* _AIController;

	FVector _targetLocation;

	TArray<FVector> _patrolPoints;

	

};
