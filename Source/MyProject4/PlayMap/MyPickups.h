// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Common/MyItem.h"
#include "MyPickups.generated.h"

UENUM(BlueprintType)
enum class EPickUpType : uint8
{
	EPT_Booster				UMETA(DisplayName = "Booster"),
	EPT_Rocket				UMETA(DisplayName = "Rocket"),
	EPT_Grab				UMETA(DisplayName = "Grab"),
	EPT_Random				UMETA(DisplayName = "Random"),
	EPT_MAX					UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class MYPROJECT4_API AMyPickups : public AMyItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyPickups();

protected:
	virtual void BeginPlay();

public:	

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	UFUNCTION(Server, Reliable)
		void RPCAdd(AMyCharacter* OtherActor);

	UFUNCTION(NetMulticast, Reliable)
		void Activate();

	UFUNCTION(Server, Reliable)
		void RestoreSelf();

	//Make these into private type later. public for debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float _effectTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float _effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
		EPickUpType _ePickUpType;


private:


	class AMyCharacter* _character;

	FTimerHandle _timerHandle;

	FTimerHandle _delayHandle;

	class AMyPlayerState* _playerState;

	TSubclassOf<AActor> _grabProjectile;

};
