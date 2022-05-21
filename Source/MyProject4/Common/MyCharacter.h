// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyGameInstance.h"
#include "MyCharacter.generated.h"


UCLASS(BlueprintType, Blueprintable)
class MYPROJECT4_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	friend class UMyCharacterActionComponent;

	friend class UMyReplicatorComponent;

	friend class UMyCharacterParkourComponent;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void Landed(const FHitResult& Hit);

	virtual void OnJumped();
	//Initial Function
	

	UFUNCTION(Client, Reliable,BlueprintCallable)
		void SetController_Client(bool signal);

	UFUNCTION(Server,Reliable)
	void SpawnDefaultWeapon();
	
	//setter getter
	UFUNCTION(BlueprintCallable)
	void SetActionComponent(UMyCharacterActionComponent* ac) {_actionComponent = ac;}
	
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
		inline UMyCharacterParkourComponent* GetCharacterParkourComponent() { return _parkourComponent; }



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


	void FlyUp_AllMighty(float value);

	void MoveForward_AllMighty(float value);

	void MoveRight_AllMighty(float value);


	UFUNCTION(Server, Reliable)
		void CorrectLocation_Server(const FVector& loc);

	UFUNCTION(NetMulticast, Reliable)
		void CorrectLocation_Multi(const FVector& loc);

	UFUNCTION(Server, Reliable)
		void SetSpeed_Server(const float& speed);

	UFUNCTION(NetMulticast, Reliable)
		void SetSpeed_Multi(const float& speed);


	void AllMightyModeBinding();


// public UPROPERTY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	UCameraComponent* _camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	USpringArmComponent* _springArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	UStaticMeshComponent* _glide;


	UPROPERTY()
	AMyPlayerState* _playerState;

	UPROPERTY(VisibleAnyWhere,BlueprintReadWrite)
		UMyCharacterParkourComponent* _parkourComponent;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite)
		UMyCharacterActionComponent* _actionComponent;
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite)
		UMyCharacterReplicatorComponent* _replicatorComponent;
	UPROPERTY()
		UCharacterMovementComponent* _characterMovementComponent;


	UPROPERTY(replicated, BlueprintReadWrite)
		bool _parkourDelay;


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

	void Jump();

	void JumpRelease();

	void Sprint();

	void SprintRelease();

	void Ledge();
	
	void UnLedge();

	UFUNCTION(Server,Reliable)
	void DelayParkour();

	UFUNCTION(Server, Reliable)
	void ClimbWall(const FVector& vec);

	UFUNCTION(NetMulticast, Reliable)
		void ClimbWall_Multi(const FVector& vec);



	//Initial Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	TSubclassOf<class AMyWeapon> _defaultWeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Attack")
	AMyWeapon* _equippedWeapon;

	//In Game Value
	float _effect;

	FTimerHandle _timerHandle;

	AActor* _target;

	FVector _clientLocationForAllmightyMode;

	//Components
};
