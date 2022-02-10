// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyCharacterActionComponent.h"
#include "MyCharacterReplicatorComponent.generated.h"


USTRUCT()
struct FMoveState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform _transform;

	UPROPERTY()
	FCharacterMoveInfo _lastMove;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT4_API UMyCharacterReplicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyCharacterReplicatorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void SendMoveToServer(const FCharacterMoveInfo& move);

	UFUNCTION(Server, Reliable)
		void RespawnCheck();

	UFUNCTION()
		void OnRep_ServerChange();


	void EnqueueAcknowledgedMove(const FCharacterMoveInfo& move) { _unacknowledgedMoves.Add(move); }

	
	//debug
	int GetMoves() { return _unacknowledgedMoves.Num();}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Debug")
		FVector _respawn;

private:	


	void ClearUnacknowledgedMoves(const FCharacterMoveInfo& lastMove);

	UPROPERTY(ReplicatedUsing = OnRep_ServerChange)
	FMoveState _serverState;


	
	class UMyCharacterActionComponent* _actionComponent;

	TWeakObjectPtr<AMyCharacter> _owner;

	UCharacterMovementComponent* _characterMovement;

	TArray<FCharacterMoveInfo> _unacknowledgedMoves;
};
