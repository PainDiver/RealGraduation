// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyCharacterParkourComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT4_API UMyCharacterParkourComponent : public UActorComponent
{
	GENERATED_BODY()


	friend AMyCharacter;

public:	
	// Sets default values for this component's properties
	UMyCharacterParkourComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



	UFUNCTION(Server, Reliable)
		void WallRunningServerCheck(bool OnOff);

	UFUNCTION(Server, Reliable)
		void WallRunRight();

	UFUNCTION(NetMulticast, Reliable)
		void WallRunRight_Multi();

	UFUNCTION(Server, Reliable)
		void WallRunLeft();

	UFUNCTION(NetMulticast, Reliable)
		void WallRunLeft_Multi();

	UFUNCTION(Server, Reliable)
		void StopWallRunning();

	UFUNCTION(NetMulticast, Reliable)
		void StopWallRunning_Multi();
	
	UFUNCTION(Server, Reliable)
	void WallRunningJump();




	UFUNCTION(Server, Reliable)
		void LedgingServerCheck(bool onOff);

	UFUNCTION(Server, Reliable)
		void Ledge();

	UFUNCTION(NetMulticast, Reliable)
		void Ledge_Multi();

	UFUNCTION(Server, Reliable)
		void LedgeJump();

	UFUNCTION(NetMulticast, Reliable)
		void LedgeJump_Multi();

	UFUNCTION(Server, Reliable)
		void Unledge();

	UFUNCTION(NetMulticast, Reliable)
		void Unledge_Multi();


	UFUNCTION(Server, Reliable)
		void Sprint();

	UFUNCTION(NetMulticast, Reliable)
		void Sprint_Multi();


	UFUNCTION(Server, Reliable)
		void StopSprint();

	UFUNCTION(NetMulticast, Reliable)
		void StopSprint_Multi();

	void CheckGage(float deltaTime);

	void RegenerateGage(float deltaTime);

	UFUNCTION(Server,Reliable)
	void Glide();

	UFUNCTION(NetMulticast, Reliable)
		void Glide_Multi();


	UFUNCTION(Server, Reliable)
	void UnGlide();

	UFUNCTION(NetMulticast, Reliable)
		void UnGlide_Multi();



	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bIsWallRunning;

	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bIsledging;

	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bIsSprinting;

	UPROPERTY(replicated, BlueprintReadWrite)
		bool _bIsGliding;


	UPROPERTY(VisibleAnyWhere,BlueprintReadWrite)
		float _stamina;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite)
		float _fuel;


private:

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float _sprintSpeed;

	float _defaultCharacterSpeed;

	bool _regenerateDelay;

	bool _fuelRegenerateDelay;

	FVector _wallNormal;

	FTimerHandle _wallRunningLeftTimer;
	
	FTimerHandle _wallRunningRightTimer;

	FTimerHandle _ledgeTimer;


	UPROPERTY()
	class AMyCharacter* _character;

	UPROPERTY()
	class UCharacterMovementComponent* _movementComponent;

	UPROPERTY()
	class UCameraComponent* _camera;

	UPROPERTY()
	class UCapsuleComponent* _capsuleComponent;

	UPROPERTY()
	class USpringArmComponent* _springArm;


};
