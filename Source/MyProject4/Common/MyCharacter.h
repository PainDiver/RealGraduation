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

	friend class UMyCharacterActionComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Initial Function
	
	UFUNCTION(Server, Reliable)
	void SpawnDefaultWeapon();

	UFUNCTION(Server, Reliable)
		void SaveColor(const FLinearColor& color);

	UFUNCTION(Client, Reliable)
		void SetColor();

	UFUNCTION(NetMulticast, Reliable)
		void SetColor_Multi(const FLinearColor& color);



	//setter getter
	UFUNCTION(BlueprintCallable)
	void SetActionComponent(class UMyCharacterActionComponent* ac) {_actionComponent = ac;}
	
	UFUNCTION(BlueprintCallable)
	inline UMyCharacterActionComponent* GetActionComponent()const {return _actionComponent;}
	
	UFUNCTION(BlueprintCallable)
	void SetCharacterMovementComponent(UCharacterMovementComponent* mc) {_characterMovementComponent = mc;}
	
	UFUNCTION(BlueprintCallable)
	inline UCharacterMovementComponent* GetCharacterMovementComponent()const {return _characterMovementComponent;}
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerState(class AMyPlayerState* ps) {_playerState = ps;}
	
	UFUNCTION(BlueprintCallable)
	inline AMyPlayerState* GetMyPlayerState()const {return _playerState;}




// public UPROPERTY
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAceess = "true"), Category = Camera)
	class UCameraComponent* _camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	class USpringArmComponent* _springArm;


private:

	void InitializeColor();

	void InitializeInstance();

	// Movement Function
	void MoveForward(float value);

	void MoveRight(float value);

	void LookUp(float value);

	void Turn(float value);

	void JumpCharacter();

	void Use();

	void Attack();

	void Greet();

	void Ride();

	void ShowCursor();


	//Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	TSubclassOf<class AMyWeapon> _DefaultWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	AMyWeapon* _equippedWeapon;


	//In Game Value
	float _effect;

	FTimerHandle _timerHandle;

	AActor* _target;

	AMyPlayerState* _playerState;





	//Components
	UMyCharacterActionComponent* _actionComponent;

	class UMyCharacterReplicatorComponent* _replicatorComponent;

	UCharacterMovementComponent* _characterMovementComponent;
	
};