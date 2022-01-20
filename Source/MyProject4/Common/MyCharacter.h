// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UCLASS()
class MYPROJECT4_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Unreliable)
		void Respawn();

	UFUNCTION(Server, Reliable)
		void UseItem();

	UFUNCTION(Server, Reliable)
		void Attack();

	UFUNCTION(Server, Reliable)
		void Greet();

	UFUNCTION(Server,Reliable)
		void SpawnDefaultWeapon();

	UFUNCTION(Client, Unreliable)
		void SetColor();

	UFUNCTION(Server, Unreliable)
		void SaveColor(FLinearColor color);

	UFUNCTION(NetMulticast, Unreliable)
		void SetColor_Multi(FLinearColor color);




	void MoveForward(float value);

	void MoveRight(float value);

	void LookUp(float value);

	void TurnAtRate(float rate);

	void LookUpAtRate(float rate);

	void ShowCursor();

	


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAceess = "true"))
	class UCameraComponent* _camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* _springArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* _spectatorArm;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float _baseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float _baseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FVector _respawn;

	UPROPERTY(replicated,BlueprintReadWrite, Category = "Animation")
	bool _IsAttacking;

	UPROPERTY(replicated, BlueprintReadWrite, Category = "Animation")
		bool _IsGreeting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TSubclassOf<class AMyWeapon> _DefaultWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	AMyWeapon* _equippedWeapon;

	class AMyPlayerState* _playerState;

	float _effect;

	class UCharacterMovementComponent* _characterMovementComponent;
	
	FTimerHandle _timerHandle;

	
};
