// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterReplicatorComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMyCharacterReplicatorComponent::UMyCharacterReplicatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	_respawn = FVector(2770.421387f, 19.757507f, 120.f);
	// ...
}


// Called when the game starts
void UMyCharacterReplicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = Cast<AMyCharacter>(GetOwner());
	_actionComponent = _owner->FindComponentByClass<UMyCharacterActionComponent>();
	_characterMovement = _owner->GetCharacterMovement();
	// ...
	
}


// Called every frame
void UMyCharacterReplicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMyCharacterReplicatorComponent::RespawnCheck_Implementation()
{
	if (_owner->GetActorLocation().Z < -2000 || _owner->GetActorLocation().Z > 8000)
	{
		_owner->SetActorLocation(_respawn);
		_owner->GetCharacterMovement()->Velocity = FVector(0.f);
	}
}


void UMyCharacterReplicatorComponent::SendMoveToServer_Implementation(const FCharacterMoveInfo& move)
{
	if (!_owner.IsValid() || !_actionComponent) return;
	_actionComponent->SimulateMove(move);

	_serverState._transform = _owner->GetActorTransform();
	_serverState._lastMove = move;
}

void UMyCharacterReplicatorComponent::OnRep_ServerChange()
{
	if (!_owner.IsValid()) return;


	ClearUnacknowledgedMoves(_serverState._lastMove);
	for (const auto& move : _unacknowledgedMoves)
	{
		_actionComponent->ReplayMove(move);
	}
}

void UMyCharacterReplicatorComponent::ClearUnacknowledgedMoves(const FCharacterMoveInfo& lastMove)
{
	
	TArray<FCharacterMoveInfo> moves;
	for (const auto& move : _unacknowledgedMoves)
	{
		if (move._timeStamp > lastMove._timeStamp)
		{
			moves.Add(move);
		}
	}

	_unacknowledgedMoves = moves;
}


void UMyCharacterReplicatorComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyCharacterReplicatorComponent, _serverState)
}


