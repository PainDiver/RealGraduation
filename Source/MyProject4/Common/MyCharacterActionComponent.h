// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Common/MyCharacter.h"
#include "Components/ActorComponent.h"
#include "MyCharacterActionComponent.generated.h"


USTRUCT()
struct FCharacterMoveInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		float _deltaTime;
	UPROPERTY()
		float _timeStamp;
	UPROPERTY()
		float _forwardInput;
	UPROPERTY()
		float _rotatingInput;
	UPROPERTY()
		float _rightInput;
	UPROPERTY()
		float _lookUpInput;
	UPROPERTY()
		bool _jumpInput;
	UPROPERTY()
		bool _useInput;
	UPROPERTY()
		bool _attackInput;
	UPROPERTY()
		bool _greetInput;
	UPROPERTY()
		bool _rideInput;

};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT4_API UMyCharacterActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMyCharacterActionComponent();

	friend AMyCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
		void ObjectScan();

	UFUNCTION(NetMulticast, Reliable)
		void Ride(AActor* target, bool Input);

	UPROPERTY(replicated, BlueprintReadWrite, Category = "Animation")
		bool _bIsAttacking;

	UPROPERTY(replicated, BlueprintReadWrite, Category = "Animation")
		bool _bIsGreeting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Debug")
		FVector _respawn;

	void SimulateMove(const FCharacterMoveInfo& MoveInfo);

	void ReplayMove(const FCharacterMoveInfo& MoveInfo);


	FCharacterMoveInfo CreateMove(const float& DeltaTime);

private:


	
	
	void RespawnCheck();

	void UseItem(bool Input);

	void Attack(bool Input);

	void Greet(bool Input);

	void AddMoveForward(const float& DeltaTime, const float& Input);

	void AddMoveRight(const float& DeltaTime, const float& Input);

	void AddLookUp(const float& DeltaTime, const float& Input);

	void AddRotation(const float& DeltaTime, const float& Input);

	void Jump(const bool& Input);


	void SetForwardInput(float value) { _forwardInput=value; }
	void SetRightInput(float value) { _rightInput= value; }
	void SetRotatingInput(float value) { _rotatingInput = value; }
	void SetLookUpInput(float value) { _lookUpInput = value; }
	void SetJumpInput(bool value) { _jumpInput = value; }
	void SetUseInput(bool value) { _useInput = value; }
	void SetAttackInput(bool value) { _attackInput = value; }
	void SetGreetInput(bool value) { _greetInput = value; }
	void SetRideInput(bool value) { _rideInput = value; }

	//Inputs
	float _forwardInput;

	float _rightInput;

	float _rotatingInput;

	float _lookUpInput;

	bool _jumpInput;

	bool _useInput;

	bool _attackInput;

	bool _greetInput;

	bool _rideInput;

	TWeakObjectPtr<AMyCharacter> _owner;
};