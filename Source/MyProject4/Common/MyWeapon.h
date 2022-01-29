// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyItem.h"
#include "MyWeapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Pickup UMETA(DisplayName = "Pickup"),
	EWS_Equip UMETA(DisplayName = "Equipped"),
	EWS_MAX UMETA(DisplayName = "DefaultMax")

};

UENUM(BlueprintType)
enum class EWeaponKind :uint8
{
	EWK_Fist UMETA(DisplayName = "Fist"),
	EWK_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class MYPROJECT4_API AMyWeapon : public AMyItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)override;
	//
	//virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)override;

	UFUNCTION()
		void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Reliable)
		void LaunchServer(AActor* OtherActor,const EWeaponKind& kind);
	
	void Equip(class AMyCharacter* character);


	void SetWeaponName(EWeaponKind kind) { _eWeaponName = kind; }
	inline EWeaponKind GetWeaponName()const { return _eWeaponName; }

	void SetWeaponState(EWeaponState state) {_eWeaponState = state; }
	inline EWeaponState GetWeaponState()const { return _eWeaponState; }


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
	class UBoxComponent* _combatCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* _skeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	EWeaponState _eWeaponState;

	UPROPERTY(VisibleAnywhere)
	EWeaponKind _eWeaponName;

};
