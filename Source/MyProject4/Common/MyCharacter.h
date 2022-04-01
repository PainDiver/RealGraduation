// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyGameInstance.h"
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
	
	UFUNCTION(Server,Reliable)
	void SpawnDefaultWeapon();
	

	//setter getter
	UFUNCTION(BlueprintCallable)
	void SetActionComponent(class UMyCharacterActionComponent* ac) {_actionComponent = ac;}
	
	UFUNCTION(BlueprintCallable)
	inline UMyCharacterActionComponent* GetActionComponent() {return _actionComponent;}
	
	UFUNCTION(BlueprintCallable)
	void SetCharacterMovementComponent(UCharacterMovementComponent* mc) {_characterMovementComponent = mc;}
	
	UFUNCTION(BlueprintCallable)
	inline UCharacterMovementComponent* GetCharacterMovementComponent() {return _characterMovementComponent;}

	UFUNCTION(BlueprintCallable)
		void SetCharacterReplicatorComponent(UMyCharacterReplicatorComponent* rc) { _replicatorComponent = rc; }

	UFUNCTION(BlueprintCallable)
		inline UMyCharacterReplicatorComponent* GetCharacterReplicatorComponent() { return _replicatorComponent; }


	UFUNCTION(BlueprintCallable)
	void SetPlayerState(class AMyPlayerState* ps) {_playerState = ps;}
	
	UFUNCTION(BlueprintCallable)
	inline AMyPlayerState* GetMyPlayerState()const {return _playerState;}


	UFUNCTION(Server,Reliable,BlueprintCallable)
		void SetColor(const FLinearColor& color);

	UFUNCTION(Client, Reliable, BlueprintCallable)
		void SetColor_Client(const FLinearColor& color);


	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void SetColor_Multi(const FLinearColor& color);



	UFUNCTION(Server, Reliable)
		void FlyUp_AllMighty_Server(float value);

	UFUNCTION(NetMulticast, Reliable)
		void FlyUp_AllMighty_Multi(float value);


	UFUNCTION(Server, Reliable)
		void MoveForward_AllMighty_Server(float value);

	UFUNCTION(NetMulticast, Reliable)
		void MoveForward_AllMighty_Multi(float value);


	UFUNCTION(Server, Reliable)
		void MoveRight_AllMighty_Server(float value);

	UFUNCTION(NetMulticast, Reliable)
		void MoveRight_AllMighty_Multi(float value);

	void AllMightyModeBinding();

// public UPROPERTY
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAceess = "true"), Category = Camera)
	class UCameraComponent* _camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	class USpringArmComponent* _springArm;


	UPROPERTY()
	AMyPlayerState* _playerState;


	UPROPERTY()
		UMyCharacterActionComponent* _actionComponent;
	UPROPERTY()
		class UMyCharacterReplicatorComponent* _replicatorComponent;
	UPROPERTY()
		UCharacterMovementComponent* _characterMovementComponent;

	UPROPERTY()
		class UMySaveGame* _saveGame;


private:


	void InitializeInstance();

	// Movement Function
	void MoveForward(float value);

	void MoveRight(float value);

	void LookUp(float value);

	void Turn(float value);

	void Interact();

	void Attack();

	void Greet();

	void ShowCursor();

	//Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	TSubclassOf<class AMyWeapon> _defaultWeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	AMyWeapon* _equippedWeapon;

	//In Game Value
	float _effect;

	FTimerHandle _timerHandle;

	AActor* _target;

	//Components
};
